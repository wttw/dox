#include "dns_dot.h"

#include "dnswire.h"

#include <QUrl>
#include <QSslSocket>
#include <QAbstractSocket>
#include <QTimer>
#include <QtDebug>

/*
 * DNS over TLS implementation for dnslookup
 */

DotImplementation::DotImplementation(ushort type, const QString& name, const QString& url, QObject *parent) : DnsImplementation(parent), request(WireRequest(type, name)), resolverUrl(url) {
}

QString DotImplementation::ErrorString() const {
    return errorString;
}

QList<QHostAddress> DotImplementation::Addresses() const {
    if (responseLength() != countedResponse.length() - 2) {
        return QList<QHostAddress>();
    }
    return WireToAddresses(countedResponse.mid(2));
}

QString DotImplementation::PrettyResults() const {
    if (responseLength() != countedResponse.length() - 2) {
        return QString();
    }
    return WireToPretty(countedResponse.mid(2));
}

void DotImplementation::lookup() {
    if (request.isEmpty()) {
        errorString = tr("Failed to serialize query");
        emit finished();
        return;
    }

    QUrl u(resolverUrl);
    if (!u.isValid() || u.scheme() != "dot" || u.host().isEmpty()) {
        errorString = tr("'%1' doesn't look like the name of a dns over TLS resolver").arg(resolverUrl);
        emit finished();
        return;
    }

    int port = u.port(853);

    connect(&sock, static_cast<void(QSslSocket::*)(const QList<QSslError> &)>(&QSslSocket::sslErrors),
            [=](const QList<QSslError> &errors){
        qWarning() << "Ignoring TLS errors: " << errors;
        sock.ignoreSslErrors();
    });

    connect(&sock, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error),
          [=](QAbstractSocket::SocketError /*socketError*/){
        errorString = tr("Socket error connecting to '%1'%2").arg(resolverUrl, sock.errorString());
        emit finished();
    });

    connect(&sock, &QSslSocket::encrypted, [=](){
       QByteArray countedRequest;
       int requestLength = request.length();
       Q_ASSERT(requestLength < 65536);

       countedRequest.append(static_cast<unsigned char>((requestLength & 0xff00) >> 8));
       countedRequest.append(static_cast<unsigned char>(requestLength & 0xff));
       countedRequest.append(request);
       timer.start();
       sock.write(countedRequest);
    });

    connect(&sock, &QSslSocket::readyRead, [=](){
        while(sock.bytesAvailable() > 0) {
            int bytesWanted;
            if (countedResponse.length() < 2) {
                bytesWanted = 2 - countedResponse.length();
            } else {
                bytesWanted = responseLength() - (countedResponse.length() - 2);
            }
            Q_ASSERT(bytesWanted > 0 && bytesWanted < 65537);
            QByteArray chunk = sock.read(bytesWanted);
            countedResponse.append(chunk);
            if (responseLength() == countedResponse.length() - 2) {
                responseTime = timer.nsecsElapsed();
                emit finished();
                return;
            }
        }
    });

    QTimer::singleShot(15000, this, [=](){
        if (sock.isOpen()) {
            sock.close();
            errorString = tr("timed out after 15 seconds");
            emit finished();
        }
    });

    sock.connectToHostEncrypted(u.host(), port);
}

int DotImplementation::responseLength() const {
    if (countedResponse.length() < 2) {
        return -1;
    }

    return static_cast<unsigned char>(countedResponse.at(0)) * 256 + static_cast<unsigned char>(countedResponse.at(1));
}
