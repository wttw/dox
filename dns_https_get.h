#ifndef DOX_DNS_HTTPS_GET_H
#define DOX_DNS_HTTPS_GET_H

#include "dns.h"

#include <QByteArray>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <QElapsedTimer>

class DohGetImplementation : public DnsImplementation {
    Q_OBJECT
public:
    DohGetImplementation(ushort type, const QString& name, const QString& url, QObject *parent=0);

    QString ErrorString() const;
    QList<QHostAddress> Addresses() const;
    QString PrettyResults() const;
    void lookup();
private:
    QByteArray request;
    QString resolverUrl;
    QString errorString;
    QByteArray response;
    QElapsedTimer timer;
};

#endif
