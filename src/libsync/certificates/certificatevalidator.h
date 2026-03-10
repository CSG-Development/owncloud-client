#pragma once

#include "personalcloudlib.h"

#include <QObject>
#include <QtNetwork/QSslCertificate>

#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509_vfy.h>

class APPLICATIONSYNC_EXPORT CertificateValidator: public QObject
{
    Q_OBJECT

public:
    explicit CertificateValidator(QObject* parent = nullptr);

    bool validatePinnedCertificate(const QList<QSslCertificate> &serverChain);

private:
    void loadPinnedCertificates();
    bool localPinnedTrustPasses(const QList<QSslCertificate> &serverChain);
    QString getFingerprint(const QSslCertificate &cert);
    bool verifySignature(const QSslCertificate &child, const QSslCertificate &issuer);

    QList<QSslCertificate> _pinned;
};
