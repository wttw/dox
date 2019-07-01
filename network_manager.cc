#include "network_manager.h"

#include <QNetworkAccessManager>
#include <QList>
#include <QSslError>
#include <QNetworkReply>
#include <QObject>
#include <QtDebug>

QNetworkAccessManager *networkManager = nullptr;

void initNetworkManager(QObject *parent) {
    Q_ASSERT(networkManager == nullptr);
    networkManager = new QNetworkAccessManager(parent);

    QObject::connect(networkManager, &QNetworkAccessManager::sslErrors, [](QNetworkReply *reply, const QList<QSslError> &errors){
        for (auto &error : errors) {
            qWarning() << "Ignoring SSL error: " << error.errorString();
        }
        reply->ignoreSslErrors();
    });
}
