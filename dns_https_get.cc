#include "dns_https_get.h"

#include "dnswire.h"
#include "network_manager.h"
#include "uritemplate.h"

#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>

using UriTemplate = uritemplatecpp::UriTemplate;

DohGetImplementation::DohGetImplementation(ushort type, const QString& name, const QString& url, QObject *parent) : DnsImplementation(parent), request(WireRequest(type, name)), resolverUrl(url) {
}

QString DohGetImplementation::ErrorString() const {
    return errorString;
}

QList<QHostAddress> DohGetImplementation::Addresses() const {
    return WireToAddresses(response);
}

QString DohGetImplementation::PrettyResults() const {
    return WireToPretty(response);
}

void DohGetImplementation::lookup() {
    if (request.isEmpty()) {
        errorString = tr("Failed to serialize query");
        emit finished();
        return;
    }

    QByteArray b64request = request.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);

    UriTemplate uri(resolverUrl.toStdString());
    uri.set("dns", b64request.toStdString());
    QString targetUrl = QString::fromStdString(uri.build());

    QUrl u(targetUrl);
    if (!u.isValid() || u.host().isEmpty()) {
        errorString = tr("'%1' doesn't look like the name of a dns over https resolver").arg(resolverUrl);
        emit finished();
        return;
    }

    QNetworkRequest req;
    req.setUrl(u);
    req.setRawHeader("Accept", "application/dns-message");

    timer.start();
    QNetworkReply *repl = networkManager->get(req);
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
