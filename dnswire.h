#ifndef DOX_DNSWIRE_H
#define DOX_DNSWIRE_H

#include <QByteArray>
#include <QString>
#include <QList>
#include <QHostAddress>

QByteArray WireRequest(ushort type, const QString& name);
QString WireToPretty(const QByteArray &wire);
QList<QHostAddress> WireToAddresses(const QByteArray &wire);

#endif
