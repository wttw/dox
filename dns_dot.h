#ifndef DOX_DNS_DOT_H
#define DOX_DNS_DOT_H

#include "dns.h"

#include <QByteArray>
#include <QString>
#include <QSslSocket>
#include <QElapsedTimer>


class DotImplementation : public DnsImplementation {
    Q_OBJECT
public:
    DotImplementation(ushort type, const QString& name, const QString& url, QObject *parent=0);

    QString ErrorString() const;
    QList<QHostAddress> Addresses() const;
    QString PrettyResults() const;
    void lookup();
private:
    QByteArray request;
    QString resolverUrl;
    QString errorString;
    QSslSocket sock;
    QByteArray countedResponse;
    int responseLength() const;
    QElapsedTimer timer;
};

#endif
