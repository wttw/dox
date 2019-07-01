#ifndef DOX_DNS_SYSTEM_H
#define DOX_DNS_SYSTEM_H

#include "dns.h"

#include <QString>
#include <QList>
#include <QHostAddress>
#include <QElapsedTimer>

class QDnsLookup;

class SystemImplementation : public DnsImplementation {
    Q_OBJECT
  public:
    SystemImplementation(ushort type, const QString& name, QObject *parent=0);

    QString ErrorString() const;
    QList<QHostAddress> Addresses() const;
    QString PrettyResults() const;
    void lookup();

private:
    QDnsLookup *dl;
    QString errorString;
    QElapsedTimer timer;
};

#endif
