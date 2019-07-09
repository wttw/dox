#include "survey.h"
#include "dns.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QFile>
#include <QTextEdit>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QElapsedTimer>
#include <QStringBuilder>
#include <QFileDialog>
#include <QMessageBox>

Survey::Survey(QWidget *parent) : QWidget(parent) {
    auto layout = new QVBoxLayout;

    overview = new QLabel;
    layout->addWidget(overview);

    stats = new QLabel;
    layout->addWidget(stats);

    progress = new QProgressBar;
    layout->addWidget(progress);
    progress->hide();

    auto buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    go = new QPushButton(tr("Go"));
    buttonLayout->addWidget(go);
    layout->addLayout(buttonLayout);

    report = new QTextEdit;
    report->setReadOnly(true);
    layout->addWidget(report, 1);
    setLayout(layout);

    connect(go, &QPushButton::clicked, this, &Survey::run);

    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::sslErrors, [=](QNetworkReply *reply, const QList<QSslError> &errors){
        for (auto &error : errors) {
            qWarning() << "Ignoring SSL error from " << reply->request().url().toDisplayString() << ": " << error.errorString();
        }
        reply->ignoreSslErrors();
    });

    // TODO(steve): redirects?

    QString err = loadFile(":/defaultSurvey.json");
    Q_ASSERT(err.isEmpty());
}

void Survey::run() {
    queue.clear();
    for (auto &r : data.resolvers) {
        for (auto &t : data.targets) {
//            qDebug() << "building queue " << t.url << r.url;
            queue.append(SurveyResult(t.url, r.url));
        }
    }
    queuePoint = 0;
    progress->setRange(0, queue.size());
    progress->show();
    report->clear();
    surveyOne();
}

void Survey::surveyOne() {
    if (queuePoint >= queue.size()) {
        // close down, display, etc
        progress->hide();
        data.results = queue;
        report->setText(richText());
        stats->setText(tr("%1 resolvers and %2 targets").arg(data.resolvers.size()).arg(data.targets.size()));
        return;
    }

    SurveyResult &current = queue[queuePoint];

    QString resolver = current.resolver;
    QString target = current.target;
    qDebug() << "surveyOne" << resolver << target;
    progress->setValue(queuePoint);
    stats->setText(tr("(%1/%2) %3 - %4").arg(queuePoint).arg(queue.size()).arg(data.resolverName.value(resolver), data.targetName.value(target)));
    queuePoint++;
    QUrl u(target);
    if (!u.isValid()) {
        qWarning() << "invalid target url" << target;
        QTimer::singleShot(0, this, &Survey::surveyOne);
        return;
    }
    auto dnsa = new DnsLookup("A", u.host(), resolver);
    auto dnsaaaa = new DnsLookup("AAAA", u.host(), resolver);

    // Starts to feel like javascript, doesn't it?
    connect(dnsa, &DnsLookup::finished, [=,&current](){
        if (dnsa->hasError()) {
            qWarning() << resolver << " A query for " << u.host() << ": " << dnsa->ErrorString();
        }

        current.addresses.append(dnsa->Addresses());
        current.rttA = dnsa->responseTime();
        dnsa->deleteLater();
        dnsaaaa->lookup();
    });

    connect(dnsaaaa, &DnsLookup::finished, [=,&current](){
        if (dnsaaaa->hasError()) {
            qWarning() << resolver << " AAAA query for " << u.host() << ": " << dnsaaaa->ErrorString();
        }
        current.addresses.append(dnsaaaa->Addresses());
        current.rttAAAA =  dnsaaaa->responseTime();
        dnsaaaa->deleteLater();
        if (current.addresses.isEmpty()) {
            qWarning() << resolver << " no A or AAAA for " << u.host();
            QTimer::singleShot(0, this, &Survey::surveyOne);
            return;
        }


        // The network access manager will cache and reuse
        // connections, so we'll clear it to make our
        // measurements fair.

        manager->clearConnectionCache();

        // We're using the regular Qt http client, which
        // wants to do it's own DNS. We'll give it a URL
        // with an IP address, but set the host header to
        // the original.

        QUrl ipUrl(u);

        // We use the first address we've stored, which'll
        // be IPv4 if there are any A records.
        ipUrl.setHost(current.addresses.first().toString());

        QNetworkRequest request(ipUrl);
        // TODO(steve): Host i18n
        request.setRawHeader("Host", u.host().toLatin1());

        QElapsedTimer timer;
        timer.start();
        QNetworkReply *reply = manager->get(request);
        connect(reply, &QNetworkReply::finished, [=,&current]() {
            current.rttFetch = timer.nsecsElapsed();
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "Error from " << u.toDisplayString() << ": " << reply->errorString();
            } else {
                auto contents = reply->readAll();
                current.responseSize = contents.size();
                qDebug() << contents;
            }
            current.responseError = reply->error();
            reply->deleteLater();
            QTimer::singleShot(0, this, &Survey::surveyOne);
        });
    });
    dnsa->lookup();
}

// Import survey data from file
QString Survey::loadFile(const QString &filename) {
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return tr("Failed to open %1: %2").arg(filename, f.errorString());
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (doc.isNull()) {
        return tr("Failed to parse %1: %2").arg(filename, err.errorString());
    }

    data.fromJson(doc.object());
    overview->setText(data.meta.value("overview").toString());
    stats->setText(tr("%1 resolvers and %2 targets").arg(data.resolvers.size()).arg(data.targets.size()));
    report->setText(richText());
    return QString();
}

QString Survey::richText() const {
    QString ret = "<style>td {padding-left: 4px; padding-right: 4px; }</style>\n";

    QMap<QString,QList<SurveyResult> > bytarget;
    for (auto &result : data.results) {
        bytarget[result.target].append(result);
    }

    for (auto &target : data.targets) {
        QList<SurveyResult> r = bytarget.value(target.url);
        if (r.isEmpty()) {
            continue;
        }
        ret = ret % tr("<h2>%1</h2>\n<table>").arg(target.name);

        qSort(r.begin(), r.end(), [](SurveyResult a, SurveyResult b) -> bool { return a.rttFetch < b.rttFetch; });
        ret = ret % tr("<tr><th>Resolver</th><th>Address</th><th>A</th><th>AAAA</th><th>Download</th></tr>\n");
        for (auto &result : r) {
            ret = ret % tr("<tr><td>%1</td><td>%2</td><td>%3 ms</td><td>%4 ms</td><td>%5 ms</td></tr>\n").arg(data.resolverName.value(result.resolver)).arg(result.addresses.front().toString()).arg(result.rttA / 1000000.0, 0, 'f', 3).arg(result.rttAAAA / 1000000.0, 0, 'f', 3).arg(result.rttFetch / 1000000.0, 0, 'f', 3);
        }
        ret = ret % "</table>\n";
    }

    return ret;
}

void Survey::load() {
    QString filename = QFileDialog::getOpenFileName(this, tr("Load Survey"), QString(), tr("JSON Files (*.json)"));
    if (!filename.isEmpty()) {
        QString err = loadFile(filename);
        if (!err.isEmpty()) {
            QMessageBox::warning(this, tr("Failed to load survey"), err);
            return;
        }
    }
}

void Survey::save() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Resolvers"), QString("resolvers.json"));
    if (!filename.isEmpty()) {
        QJsonDocument doc(data.toJson().toObject());
        QFile f(filename);
        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(this, tr("Failed to save survey"), tr("Failed to create %1: %2").arg(filename, f.errorString()));
        }
        f.write(doc.toJson(QJsonDocument::Indented));
    }
}
