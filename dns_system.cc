#include "dns_system.h"

#include <QDnsLookup>

SystemImplementation::SystemImplementation(ushort type, const QString &name, QObject *parent) : DnsImplementation(parent) {
    switch (type) {
    case QDnsLookup::A:
    case QDnsLookup::AAAA:
    case QDnsLookup::CNAME:
    case QDnsLookup::MX:
    case QDnsLookup::NS:
    case QDnsLookup::PTR:
    case QDnsLookup::SRV:
    case QDnsLookup::TXT:
        dl = new QDnsLookup(static_cast<QDnsLookup::Type>(type), name, this);
        connect(dl, &QDnsLookup::finished, this, [=](){
            responseTime = timer.nsecsElapsed();
            qDebug() << "QDnsLookup finished";
            emit finished();
        });
        break;
    default:
        dl = nullptr;
        errorString = tr("Qt resolver doesn't handle queries of type %1").arg(type);
    }
}

QString SystemImplementation::ErrorString() const {
    if (!errorString.isEmpty()) {
        return errorString;
    }
    Q_ASSERT(dl != nullptr);
    return dl->errorString();
}

QList<QHostAddress> SystemImplementation::Addresses() const {
    QList<QHostAddress> ret;
    if (dl == nullptr) {
        return ret;
    }
    for (auto &i : dl->hostAddressRecords()) {
        ret.append(i.value());
    }
    return ret;
}

void SystemImplementation::lookup() {
    if (dl != nullptr) {
        timer.start();
        dl->lookup();
        return;
    }
    emit finished();
}

QString SystemImplementation::PrettyResults() const {
    QString error = ErrorString();
    if (!error.isEmpty()) {
        return error;
    }

    QStringList ret;
    for (auto &rr : dl->canonicalNameRecords()) {
         ret.append(QString("%1 %2 CNAME %3").arg(rr.name()).arg(rr.timeToLive()).arg(rr.value()));
    }
    for (auto &rr : dl->hostAddressRecords()) {
        switch (rr.value().protocol()) {
        case QAbstractSocket::IPv4Protocol:
            ret.append(QString("%1 %2 A %3").arg(rr.name()).arg(rr.timeToLive()).arg(rr.value().toString()));
            break;
        case QAbstractSocket::IPv6Protocol:
            ret.append(QString("%1 %2 AAAA %3").arg(rr.name()).arg(rr.timeToLive()).arg(rr.value().toString()));
            break;
        default:
            ret.append(tr("Impossible protocol for %1: %2").arg(rr.name()).arg(rr.value().protocol()));
            break;
        }
    }
    for (auto &rr : dl->mailExchangeRecords()) {
        ret.append(QString("%1 %2 MX %3 %4").arg(rr.name()).arg(rr.timeToLive()).arg(rr.preference()).arg(rr.exchange()));
    }
    for (auto &rr : dl->nameServerRecords()) {
        ret.append(QString("%1 %2 NS %3").arg(rr.name()).arg(rr.timeToLive()).arg(rr.value()));
    }
    for (auto &rr : dl->pointerRecords()) {
        ret.append(QString("%1 %2 PTR %3").arg(rr.name()).arg(rr.timeToLive()).arg(rr.value()));
    }
    for (auto &rr : dl->serviceRecords()) {
        ret.append(QString("%1 %2 SRV %3 %4 %5 %6").arg(rr.name()).arg(rr.timeToLive()).arg(rr.priority()).arg(rr.weight()).arg(rr.port()).arg(rr.target()));
    }
    for (auto &rr : dl->textRecords()) {
        QStringList vals;
        for (auto &piece : rr.values()) {
            // TODO(steve): fix escaping
            vals.append(QString("\"%1\"").arg(QString::fromLatin1(piece).replace("\\", "\\\\").replace("\"", "\\\"")));
        }
        ret.append(QString("%1 %2 TXT %3").arg(rr.name()).arg(rr.timeToLive()).arg(vals.join(" ")));
    }

    return ret.join("\n");
}

