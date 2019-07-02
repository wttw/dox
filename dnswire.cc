#include "dnswire.h"

#include "dns-storage.hh"
#include "dnsmessages.hh"

// We use tdns from https://github.com/ahupowerdns/hello-dns
// for serialization of DNS queries and replies, but this
// file is the only place it appears.

QByteArray WireRequest(ushort type, const QString &name) {
    // TODO(steve): Handle IDN
    DNSName dn = makeDNSName(name.toStdString());
    DNSType dt = static_cast<DNSType>(type);
    DNSMessageWriter dmw(dn, dt);
    dmw.dh.rd = true;
    dmw.randomizeID();
    dmw.setEDNS(4000, false);
    return QByteArray::fromStdString(dmw.serialize());
}



QString WireToPretty(const QByteArray &wire) {
    DNSMessageReader dmr(wire.toStdString());
    DNSSection rrsection, lastsection = DNSSection::Question;
    uint32_t ttl;
    DNSName dn;
    DNSType dt;
    dmr.getQuestion(dn, dt);
    std::ostringstream str;

    str << "; Question section\n" << dn << "\t" << dt << "\n";
    std::unique_ptr<RRGen> rr;
    while(dmr.getRR(rrsection, dn, dt, ttl, rr)) {
        if (rrsection != lastsection) {
            str << "; " << rrsection << " section\n";
            lastsection = rrsection;
        }
        str << dn << "\t" << dt << "\t" << ttl << "\t" << rr->toString() << "\n";
    }

    return QString::fromStdString(str.str());
}

QList<QHostAddress> WireToAddresses(const QByteArray &wire) {
    if (wire.isEmpty()) {
        return QList<QHostAddress>();
    }
    QList<QHostAddress> ret;

    DNSMessageReader dmr(wire.toStdString());
    DNSSection rrsection;
    uint32_t ttl;
    DNSName dn;
    DNSType dt;
    std::unique_ptr<RRGen> rr;

    while (dmr.getRR(rrsection, dn, dt, ttl, rr)) {
        if (rrsection != DNSSection::Answer) {
            continue;
        }
        qDebug() << "  answer";
        switch (rr->getType()) {
        case DNSType::A:
        case DNSType::AAAA:
            ret.append(QHostAddress(QString::fromStdString(rr->toString())));
            break;
        default:
            break;
        }
    }

    return ret;
}
