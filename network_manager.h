#ifndef DOX_NETWORK_MANAGER_H
#define DOX_NETWORK_MANAGER_CC

#include <QObject>
class QNetworkAccessManager;

extern QNetworkAccessManager *networkManager;

void initNetworkManager(QObject *parent = nullptr);
#endif
