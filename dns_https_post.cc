#include "dns_https_post.h"

#include "dnswire.h"
#include "network_manager.h"

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>


DohPostImplementation::DohPostImplementation(ushort type, const QString& name, const QString& url, QObject *parent) : DnsImplementation(parent), request(WireRequest(type, name)), resolverUrl(url) {
}

QString DohPostImplementation::ErrorString() const {
    return errorString;
}

QList<QHostAddress> DohPostImplementation::Addresses() const {
    return WireToAddresses(response);
}

QString DohPostImplementation::PrettyResults() const {
    return WireToPretty(response);
}

void DohPostImplementation::lookup() {
    if (request.isEmpty()) {
        errorString = tr("Failed to serialize query");
        emit finished();
        return;
    }
    QUrl u(resolverUrl);
    if (!u.isValid() || u.host().isEmpty()) {
        errorString = tr("'%1' doesn't look like the name of a dns over https resolver").arg(resolverUrl);
        emit finished();
        return;
    }

    QNetworkRequest req;
    req.setUrl(u);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/dns-message");
    req.setRawHeader("Accept", "application/dns-message");
    timer.start();
    QNetworkReply *repl = networkManager->post(req, request);
    connect(repl, &QNetworkReply::encrypted, this, [=](){
        timer.start();
    });
    connect(repl, &QNetworkReply::finished, this, [=](){
        responseTime = timer.nsecsElapsed();
        if (repl->error() != QNetworkReply::NoError) {
            errorString = repl->errorString();
        } else {
            QVariant ct = repl->header(QNetworkRequest::ContentTypeHeader);
            if (ct.isValid() && !ct.toString().toLower().contains("application/dns-message")) {
                errorString = tr("Unexpected content-type: %1").arg(ct.toString());
            } else {
                response = repl->readAll();
            }
        }
        emit finished();
        repl->deleteLater();
    });
}
