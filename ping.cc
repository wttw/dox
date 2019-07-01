#include "ping.h"

#include "dns.h"
#include "resolvers.h"

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QFormLayout>
#include <QList>
#include <QHostAddress>
#include <QTcpSocket>
#include <QElapsedTimer>

/*
 * A panel that will look up a site via a given resolver
 * then time the round trip for a tcp connection to it.
 */
Ping::Ping(Resolvers *res, QWidget *parent) : QWidget(parent), resolvers(res), sock(nullptr) {
    auto inputForm = new QFormLayout;

    resolver = new QComboBox;
    resolver->setEditable(true);
    resolver->setObjectName("query_resolver");
    connect(resolvers, &Resolvers::resolversChanged, [=](){
        resolvers->populateCombo(resolver);
    });
    resolvers->populateCombo(resolver);
    inputForm->addRow(tr("Resolver"), resolver);

    name = new QComboBox;
    name->setEditable(true);
    name->setObjectName("query_name");
    name->setInsertPolicy(QComboBox::InsertAtTop);
    name->setEditText("cnn.com");
    inputForm->addRow(tr("Target website"), name);

    auto buttonsLayout = new QHBoxLayout;

    go = new QPushButton(tr("Go"));
    connect(go, &QPushButton::clicked, this, &Ping::run);
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(go);

    results = new QTextEdit;
    results->setReadOnly(true);

    auto topLayout = new QVBoxLayout;
    topLayout->addLayout(inputForm);
    topLayout->addLayout(buttonsLayout);
    topLayout->addWidget(results, 1);
    setLayout(topLayout);

    timer = new QElapsedTimer;
}

void Ping::run() {
    stop();
    QUrl u = QUrl::fromUserInput(name->currentText());
    if (!u.isValid() || u.host().isEmpty()) {
        results->setText(tr("'%1' doesn't look like a valid target").arg(name->currentText()));
        return;
    }

    port = u.port(443);

    QString resolverUrl;

    // We pull the resolver URL from the userdata for preloaded
    // resolvers. But if the user has entered something manually
    // then we use that instead. There's no great way to detect
    // that, so we look to see if the displayed text is the same
    // as the text that was set at this index.
    if (!resolver->currentData().isNull() && resolver->currentText() == resolver->itemText(resolver->currentIndex())) {
        resolverUrl = resolver->currentData().toString();
    } else {
        resolverUrl = resolver->currentText();
    }

    auto qa = new DnsLookup("A", u.host(), resolverUrl, this);
    auto qaaaa = new DnsLookup("AAAA", u.host(), resolverUrl, this);

    results->setText(tr("Querying for %1 via %2\n\n").arg(u.host(), resolverUrl));

    connect(qa, &DnsLookup::finished, [=](){
        qaaaa->lookup();
    });

    connect(qaaaa, &DnsLookup::finished, [=](){
        auto hosts = qa->Addresses();
        hosts.append(qaaaa->Addresses());
        QStringList prettyAddresses;
        for (auto &h : hosts) {
            prettyAddresses.append(h.toString());
        }
        results->append(tr("Resolved to %1\n").arg(prettyAddresses.join(", ")));
        results->append(tr("Connecting to port %1\n\n").arg(port));

        queue = hosts;
        totalHosts = 0;
        totalTime = 0;
        dnsTime = (qa->responseTime() + qaaaa->responseTime()) / 2000000.0;
        qa->deleteLater();
        qaaaa->deleteLater();
        pingQueue();
    });
    qa->lookup();
}

void Ping::pingQueue() {
    // TODO(steve): add timeout on connect
    if (queue.isEmpty()) {
        if (totalHosts) {
            double connectTime = totalTime / (totalHosts * 1000000.0);
            results->append(tr("DNS latency: %1 Average tcp connect latency: %2 Total: %3").arg(dnsTime, 0, 'f', 3).arg(connectTime, 0, 'f', 3).arg(dnsTime + connectTime, 0, 'f', 3));
        }
        stop();
        return;
    }

    if (sock == nullptr) {
        sock = new QTcpSocket(this);
        connect(sock, &QTcpSocket::connected, this, [=](){
           auto elapsed = timer->nsecsElapsed();
           totalHosts++;
           totalTime += elapsed;
           results->moveCursor(QTextCursor::End);
           results->insertPlainText(tr("%1 ms\n").arg(elapsed / 1000000.0, 0, 'f', 3));
           sock->disconnectFromHost();
        });
        connect(sock, &QTcpSocket::disconnected, this, [=](){
           pingQueue();
        });
        connect(sock, static_cast<void(QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QAbstractSocket::error), this, [=](QAbstractSocket::SocketError /*socketError*/){
            results->moveCursor(QTextCursor::End);
            results->insertPlainText(tr("error: %1\n").arg(sock->errorString()));
            pingQueue();
        });
    }

    auto addr = queue.takeFirst();
    results->moveCursor(QTextCursor::End);
    results->insertPlainText(tr("%1: ").arg(addr.toString()));
    timer->start();
    sock->connectToHost(addr, port);
}

void Ping::stop() {
    queue.clear();
    if (sock != nullptr) {
        sock->disconnectFromHost();
    }
    timer->invalidate();
}
