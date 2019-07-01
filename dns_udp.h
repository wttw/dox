#ifndef DOX_DNS_UDP_H
#define DOX_DNS_UDP_H

#include "dns.h"

#include <QByteArray>
#include <QString>
#include <QElapsedTimer>
#include <QHostAddress>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QTimer>

class UdpImplementation : public DnsImplementation {
  Q_OBJECT
public:
    UdpImplementation(ushort type, const QString& name, const QString& url, QObject *parent=0);

    QString ErrorString() const;
    QList<QHostAddress> Addresses() const;
    QString PrettyResults() const;
    void lookup();
private:
    void start(QHostAddress server, int port);
    QByteArray request;
    QString resolverUrl;
    QString errorString;
    QNetworkDatagram response;
    QElapsedTimer timer;
    QUdpSocket sock;
    QTimer retry;
};

#endif
