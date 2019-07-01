#ifndef DOX_PING_H
#define DOX_PING_H

#include <QWidget>
#include <QList>
#include <QHostAddress>

class QComboBox;
class QPushButton;
class QTextEdit;
class QTcpSocket;
class QElapsedTimer;
class Resolvers;

class Ping : public QWidget {
    Q_OBJECT
public:
    Ping(Resolvers *res, QWidget *parent=0);
protected slots:
    void run();
    void pingQueue();
    void stop();
private:
    Resolvers *resolvers;
    int port;
    QComboBox *resolver;
    QComboBox *name;
    QPushButton *go;
    QTextEdit *results;
    QList<QHostAddress> queue;
    QTcpSocket *sock;
    QElapsedTimer *timer;
    int totalHosts;
    qint64 totalTime;
    double dnsTime;
};

#endif
