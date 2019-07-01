#ifndef DOX_DNS_H
#define DOX_DNS_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QString>

class QDnsLookup;

class DnsImplementation : public QObject {
    Q_OBJECT
public:
    DnsImplementation(QObject *parent);
    virtual QString ErrorString() const = 0;
    virtual QList<QHostAddress> Addresses() const = 0;
    virtual QString PrettyResults() const = 0;
    virtual void lookup() = 0;
    qint64 responseTime;
signals:
    void finished();

};


class BadImplementation : public DnsImplementation {
    Q_OBJECT
public:
    BadImplementation(const QString& error, QObject *parent=0);
    QString ErrorString() const { return errorString ;}
    QList<QHostAddress> Addresses() const {return QList<QHostAddress>{}; }
    QString PrettyResults() const { return ErrorString(); }
    void lookup();
private:
    QString errorString;
};

class DnsLookup : public QObject {
    Q_OBJECT
public:
    DnsLookup(const QString &type, const QString &name, const QString resolver, QObject *parent = nullptr);
    QString ErrorString() const;
    QList<QHostAddress> Addresses() const;
    QString PrettyResults();
    qint64 responseTime() { return q->responseTime; }
    bool hasError() const { !ErrorString().isEmpty(); }
signals:
    void finished();
public slots:
    void lookup();
private:
    DnsImplementation *q;
};



#endif
