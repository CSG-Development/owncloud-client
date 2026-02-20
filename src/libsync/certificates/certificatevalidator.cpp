#include "certificatevalidator.h"

#include <QCryptographicHash>
#include <QFile>
#include <QLoggingCategory>
#include <QSslError>
#include <QSslSocket>

#include <openssl/err.h>

namespace {

const QStringList certs = {
QStringLiteral(":/certificates/_.noveogroup.com.pem"),
QStringLiteral(":/certificates/_.remote.lasea.fr.pem"),
QStringLiteral(":/certificates/ca.crt"),
QStringLiteral(":/certificates/fake-device-noveo.cer"),
QStringLiteral(":/certificates/tdci.pem"),
};

QByteArray loadCertFromResource(const QString& res) {
    static QHash<QString, QByteArray> certCache;

    if (certCache.contains(res)) {
        return certCache.value(res);
    }

    QFile file(res);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray content = file.readAll();
        certCache.insert(res, content);
        return content;
    }

    return {};
}

}

Q_LOGGING_CATEGORY(lcCertValidation, "certvalidator", QtDebugMsg)

CertificateValidator::CertificateValidator(QObject *parent)
    : QObject(parent)
{
    loadPinnedCertificates();
}

bool CertificateValidator::validatePinnedCertificate(const QList<QSslCertificate> &serverChain)
{
    if (serverChain.isEmpty())
        return false;

    return localPinnedTrustPasses(serverChain, _pinned);
}

QString CertificateValidator::getFingerprint(const QSslCertificate &cert)
{
    auto s = cert.digest(QCryptographicHash::Sha256).toHex(0x3A);   // ':'
    return QString::fromLatin1(s).toUpper();
}

bool CertificateValidator::verifySignature(const QSslCertificate &child, const QSslCertificate &issuer)
{
    QByteArray childDer = child.toDer();
    QByteArray issuerDer = issuer.toDer();

    const unsigned char *pChild = reinterpret_cast<const unsigned char*>(childDer.data());
    const unsigned char *pIssuer = reinterpret_cast<const unsigned char*>(issuerDer.data());

    X509 *x509Child = d2i_X509(nullptr, &pChild, childDer.size());
    X509 *x509Issuer = d2i_X509(nullptr, &pIssuer, issuerDer.size());

    if (!x509Child || !x509Issuer) {
        if (x509Child) X509_free(x509Child);
        if (x509Issuer) X509_free(x509Issuer);
        return false;
    }

    EVP_PKEY *issuerPubKey = X509_get_pubkey(x509Issuer);

    int result = X509_verify(x509Child, issuerPubKey);

    EVP_PKEY_free(issuerPubKey);
    X509_free(x509Child);
    X509_free(x509Issuer);

    qCDebug(lcCertValidation) << "Cert verify result" << result;
    return (result == 1);
}

bool CertificateValidator::verifySelfSigned(const QSslCertificate &cert)
{
    QDateTime now = QDateTime::currentDateTime();

    if (now < cert.effectiveDate() || now > cert.expiryDate()) {
        qCDebug(lcCertValidation) << "Cert expired";
        return false;
    }

    QString certHash = getFingerprint(cert);
    bool isTrusted = false;
    for (const auto &trusted : std::as_const(_pinned)) {
        if (getFingerprint(trusted) == certHash) {
            isTrusted = true;
            break;
        }
    }

    if (!isTrusted) {
        qCDebug(lcCertValidation) << "Cert fingerprint not found in pinned list";
        return false;
    }

    return verifySignature(cert, cert);
}

void CertificateValidator::loadPinnedCertificates()
{
    _pinned.clear();

    for (const auto& s: certs) {

        const auto data = loadCertFromResource(s);

        QByteArray derData = data;
        if (data.startsWith("-----BEGIN CERTIFICATE-----")) {
            // PEM -> DER
            auto* bio = BIO_new_mem_buf(data.constData(), data.size());
            if (bio) {
                auto* x509 = PEM_read_bio_X509(bio, nullptr, nullptr, nullptr);
                if (x509) {
                    unsigned char* derBuf = nullptr;
                    int derLen = i2d_X509(x509, &derBuf);
                    if (derLen > 0 && derBuf) {
                        derData = QByteArray(reinterpret_cast<char*>(derBuf), derLen);
                        OPENSSL_free(derBuf);
                    }
                    X509_free(x509);
                }
                BIO_free(bio);
            }
        }

        QList<QSslCertificate> cert = QSslCertificate::fromData(derData, QSsl::Der);
        _pinned.append(cert);
    }
}

bool CertificateValidator::localPinnedTrustPasses(const QList<QSslCertificate> &chain, const QList<QSslCertificate> &pinnedAnchors)
{
    if (pinnedAnchors.isEmpty() || chain.isEmpty())
        return false;

    for (const auto &anchor : pinnedAnchors) {
        if (validateCertificateChain(chain, anchor)) {
            return true;
        }
    }
    return false;
}


bool CertificateValidator::validateCertificateChain(QList<QSslCertificate> chain, const QSslCertificate &anchor)
{
    if (chain.isEmpty())
        return false;

    if (chain.last().subjectDisplayName() != anchor.subjectDisplayName()) {
        chain.append(anchor);
    }

    // maybe self signed
    if (chain.size() == 1) {
        return verifySelfSigned(chain.first());
    }

    QDateTime now = QDateTime::currentDateTime();

    for (int i = 0; i < chain.size() - 1; ++i) {
        const auto &child = chain.at(i);
        const auto &issuer = chain.at(i + 1);

        //if (child.issuerDisplayName() != issuer.subjectDisplayName()) {
        //    return false;
        //}

        if (now < child.effectiveDate() || now > child.expiryDate()) {
            qCDebug(lcCertValidation) << "Cert expired";
            return false;
        }

        if (!verifySignature(child, issuer)) {
            qCDebug(lcCertValidation) << "Cert signature verification failed";
            return false;
        }

        QString anchorHash = getFingerprint(anchor);
        if (getFingerprint(child) == anchorHash || getFingerprint(issuer) == anchorHash) {
            qCDebug(lcCertValidation) << "Cert fingerprint is not match";
            return true;
        }
    }
    qCDebug(lcCertValidation) << "Cert accepted";

    return true;
}

