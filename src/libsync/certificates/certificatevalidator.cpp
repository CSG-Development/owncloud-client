#include "certificatevalidator.h"

#include <QCryptographicHash>
#include <QFile>
#include <QLoggingCategory>
#include <QSslError>
#include <QSslSocket>

#include <openssl/crypto.h>
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

using X509_ptr = std::unique_ptr<X509, decltype(&X509_free)>;
using EVP_PKEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using BIO_ptr = std::unique_ptr<BIO, decltype(&BIO_free)>;

CertificateValidator::CertificateValidator(QObject *parent)
    : QObject(parent)
{
    loadPinnedCertificates();
}

bool CertificateValidator::validatePinnedCertificate(const QList<QSslCertificate> &serverChain)
{
    if (serverChain.isEmpty())
        return false;

    return localPinnedTrustPasses(serverChain);
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

    X509_ptr x509Child(d2i_X509(nullptr, &pChild, childDer.size()), X509_free);
    X509_ptr x509Issuer(d2i_X509(nullptr, &pIssuer, issuerDer.size()), X509_free);

    if (!x509Child || !x509Issuer) {
        qCDebug(lcCertValidation) << "Failed to parse certificates using OpenSSL";
        return false;
    }

    EVP_PKEY_ptr issuerPubKey(X509_get_pubkey(x509Issuer.get()), EVP_PKEY_free);
    if (!issuerPubKey) {
        qCDebug(lcCertValidation) << "Failed to extract public key";
        return false;
    }

    int result = X509_verify(x509Child.get(), issuerPubKey.get());

    return (result == 1);
}


void CertificateValidator::loadPinnedCertificates()
{
    _pinned.clear();

    for (const auto& s: certs) {

        const auto data = loadCertFromResource(s);
        if (data.isEmpty())
            continue;

        QByteArray derData = data;

        if (data.startsWith("-----BEGIN CERTIFICATE-----")) {
            // PEM -> DER
            BIO_ptr bio(BIO_new_mem_buf(data.constData(), data.size()), BIO_free);
            if (!bio)
                continue;

            X509_ptr x509(PEM_read_bio_X509(bio.get(), nullptr, nullptr, nullptr), X509_free);
            if (!x509)
                continue;

            unsigned char* derBuf = nullptr;
            int derLen = i2d_X509(x509.get(), &derBuf);

            if (derLen > 0 && derBuf) {
                derData = QByteArray(reinterpret_cast<const char*>(derBuf), derLen);
                OPENSSL_free(derBuf);
            }
        }

        QList<QSslCertificate> cert = QSslCertificate::fromData(derData, QSsl::Der);
        _pinned.append(cert);
    }

}

bool CertificateValidator::localPinnedTrustPasses(const QList<QSslCertificate> &serverChain)
{
    if (_pinned.isEmpty() || serverChain.isEmpty())
        return false;

    QDateTime now = QDateTime::currentDateTime();

    for (const auto &serverCert : serverChain) {

        if (now < serverCert.effectiveDate() || now > serverCert.expiryDate()) {
            qCDebug(lcCertValidation) << "Server cert expired:" << serverCert.subjectDisplayName();
            continue;
        }

        QString serverFingerprint = getFingerprint(serverCert);

        for (const auto &pinnedCert : std::as_const(_pinned)) {

            if (serverFingerprint == getFingerprint(pinnedCert)) {
                qCDebug(lcCertValidation) << "Match by fingerprint:" << serverCert.subjectDisplayName();
                return true;
            }

            if (verifySignature(serverCert, pinnedCert)) {
                qCDebug(lcCertValidation) << "Signature verified:"
                                          << serverCert.subjectDisplayName()
                                          << "is signed by"
                                          << pinnedCert.subjectDisplayName();
                return true;
            }
        }
    }

    qCDebug(lcCertValidation) << "No matches found in pinned certificates";
    return false;
}

