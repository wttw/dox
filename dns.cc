#include "dns.h"

#include "dns_system.h"
#include "dns_https_get.h"
#include "dns_https_post.h"
#include "dns_dot.h"
#include "dns_udp.h"

#include <QDnsLookup>
#include <QTimer>

DnsImplementation::DnsImplementation(QObject *parent) : QObject(parent) {

}


BadImplementation::BadImplementation(const QString &error, QObject *parent) : DnsImplementation(parent), errorString(error) {

}

void BadImplementation::lookup() {
    QTimer::singleShot(0, this, &BadImplementation::finished);
}

DnsLookup::DnsLookup(const QString &queryType, const QString &name, const QString resolver, QObject *parent) : QObject(parent){
    ushort type=0;

    QString typeName = queryType.toUpper().trimmed();
    if (typeName == "A") {
        type = QDnsLookup::A;
    } else if (typeName == "AAAA") {
        type = QDnsLookup::AAAA;
    } else if (typeName == "CNAME") {
        type = QDnsLookup::CNAME;
    } else if (typeName == "MX") {
        type = QDnsLookup::MX;
    } else if (typeName == "NS") {
        type = QDnsLookup::NS;
    } else if (typeName == "PTR") {
        type = QDnsLookup::PTR;
    } else if (typeName == "SRV") {
        type = QDnsLookup::SRV;
    } else if (typeName == "TXT") {
        type = QDnsLookup::TXT;
    } else {
        type = typeName.toUShort();
    }

    if (type == 0) {
        q = new BadImplementation(tr("Bad query type: %1").arg(queryType));
    } else {

        if (resolver == "system") {
            q = new SystemImplementation(type, name, this);
        } else if (resolver.startsWith("https:")) {
            if (resolver.contains("{")) {
                q = new DohGetImplementation(type, name, resolver, this);
            } else {
                q = new DohPostImplementation(type, name, resolver, this);
            }
        } else if (resolver.startsWith("dot:")) {
            q = new DotImplementation(type, name, resolver, this);
        } else if (resolver.startsWith("udp:")) {
            q = new UdpImplementation(type, name,resolver, this);
        } else {
            QUrl u = QUrl::fromUserInput(resolver);
            if (u.isValid()) {
                q = new UdpImplementation(type, name, "udp://" + u.host() + ":" + QString::number(u.port(53)));
            } else {
                q = new BadImplementation(tr("Bad resolver: %1").arg(resolver));
            }
        }
    }
    connect(q, &DnsImplementation::finished, this, &DnsLookup::finished);
}

void DnsLookup::lookup() {
    q->lookup();
}

QString DnsLookup::ErrorString() const {
    return q->ErrorString();
}

QList<QHostAddress> DnsLookup::Addresses() const {
    return q->Addresses();
}

QString DnsLookup::PrettyResults() {
    return q->PrettyResults();
}
