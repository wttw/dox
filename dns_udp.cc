#include "dns_udp.h"

#include "dnswire.h"

#include <QUrl>
#include <QUdpSocket>
#include <QTimer>
#include <QHostInfo>
#include <QNetworkDatagram>

/*
 * Traditional DNS over UDP implementation for dnslookup
 */

UdpImplementation::UdpImplementation(ushort type, const QString& name, const QString& url, QObject *parent) : DnsImplementation(parent), request(WireRequest(type, name)), resolverUrl(url) {
}

QString UdpImplementation::ErrorString() const {
    return errorString;
}

QList<QHostAddress> UdpImplementation::Addresses() const {
    return WireToAddresses(response.data());
}

QString UdpImplementation::PrettyResults() const {
    // TODO(steve): Annotate with where the response came from
    return WireToPretty(response.data());
}

void UdpImplementation::lookup() {
    if (request.isEmpty()) {
        errorString = tr("Failed to serialize query");
        emit finished();
        return;
    }

    QUrl u = QUrl::fromUserInput(resolverUrl);
    if (!u.isValid() || u.scheme() != "udp" || u.host().isEmpty()) {
        errorString = tr("'%1' doesn't look like the name of a dns over UDP resolver").arg(resolverUrl);

        emit finished();
        return;
    }

    int port = u.port(53);

    QHostAddress addr(u.host());
    if (!addr.isNull()) {
        start(addr, port);
        return;
    }

    QHostInfo::lookupHost(u.host(), this, [=](QHostInfo h){
        if (h.error() != QHostInfo::NoError) {
            errorString = tr("Couldn't resolve %1: %2").arg(u.host(), h.errorString());
            emit finished();
            return;
        }
        if (h.addresses().isEmpty()) {
            errorString = tr("%1 doesn't exist").arg(u.host());
            emit finished();
            return;
        }
        start(h.addresses().front(), port);
    });
}
/*
 * Actually do the query
 */
void UdpImplementation::start(QHostAddress server, int port) {
    if (!sock.bind()) {
        errorString = tr("Failed to bind UDP socket: %1").arg(sock.errorString());
        emit finished();
        return;
    }

    connect(&sock, &QUdpSocket::readyRead, [=](){
        while(sock.hasPendingDatagrams()) {
            auto dg = sock.receiveDatagram();
            if (!dg.isValid()) {
                qWarning() << tr("Invalid datagram discarded");
                continue;
            }

            if (dg.data().length() < 12) {
                qWarning() << tr("Short datagram (%1) discarded").arg(dg.data().length());
                continue;
            }

            // Not sure if we're actually setting the ID
            // in the request, but still.
            if (dg.data().at(0) != request.at(0) ||
                    dg.data().at(1) != request.at(1)) {
                qWarning() << tr("Response with mismatched ID discarded");
                continue;
            }
            responseTime = timer.nsecsElapsed();
            retry.stop();
            response = dg;
            sock.close();
            emit finished();
            return;
        }
    });

    QTimer::singleShot(15000, this, [=](){
        if (sock.isOpen()) {
            sock.close();
            errorString = tr("timed out after 15 seconds");
            emit finished();
        }
    });

    connect(&retry, &QTimer::timeout, [=](){
        sock.writeDatagram(request, server, port);
    });

    timer.start();
    sock.writeDatagram(request, server, port);
    retry.start(1000); // resend the request once a second
}
