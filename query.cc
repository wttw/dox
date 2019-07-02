#include "query.h"

#include "dns.h"

#include "resolvers.h"

#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QFormLayout>
#include <QSettings>
#include <QLineEdit>

/*
 * A panel that'll lookup a DNS record via a chosen
 * resolver and display it prettily.
 */
Query::Query(Resolvers *res, QWidget *parent) : QWidget(parent), resolvers(res) {
    auto inputForm = new QFormLayout;

    resolver = new QComboBox;
    resolver->setEditable(true);
    resolver->setObjectName("query_resolver");
    connect(resolvers, &Resolvers::resolversChanged, [=](){
        resolvers->populateCombo(resolver);
    });
    resolvers->populateCombo(resolver);
    inputForm->addRow(tr("Resolver"), resolver);
    type = new QComboBox;
    type->setEditable(true);
    type->setObjectName("query_type");
    type->addItem(tr("A+AAAA"), QString("A+AAAA"));
    type->addItem(tr("A"), QString("A"));
    type->addItem(tr("AAAA"), QString("AAAA"));
    type->addItem(tr("CNAME"), QString("CNAME"));
    type->addItem(tr("MX"), QString("MX"));
    type->addItem(tr("NS"), QString("NS"));
    type->addItem(tr("PTR"), QString("PTR"));
    type->addItem(tr("SRV"), QString("SRV"));
    type->addItem(tr("TXT"), QString("TXT"));
    inputForm->addRow(tr("Query type"), type);
    name = new QComboBox;
    name->setEditable(true);
    name->setObjectName("query_name");
    name->setInsertPolicy(QComboBox::InsertAtTop);
    name->lineEdit()->setPlaceholderText(tr("e.g. github.com"));
    inputForm->addRow(tr("Query name"), name);

    auto buttonsLayout = new QHBoxLayout;

    go = new QPushButton(tr("Go"));
    connect(go, &QPushButton::clicked, this, &Query::run);
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(go);

    results = new QTextEdit;
    results->setReadOnly(true);

    auto topLayout = new QVBoxLayout;
    topLayout->addLayout(inputForm);
    topLayout->addLayout(buttonsLayout);
    topLayout->addWidget(results, 1);
    setLayout(topLayout);
    QSettings s;
    s.beginGroup("query");
    resolver->setCurrentIndex(s.value("resolver", 0).toInt());
    type->setCurrentIndex(s.value("type", 0).toInt());
    name->setEditText(s.value("name").toString());
    s.endGroup();
}

void Query::saveState() {
    QSettings s;
    s.beginGroup("query");
    s.setValue("resolver", resolver->currentIndex());
    s.setValue("type", type->currentIndex());
    s.setValue("name", name->currentText());
    s.endGroup();
}

void Query::run() {
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

    QString queryType = type->currentText();
    if (!type->currentData().isNull()) {
        queryType = type->currentData().toString();
    }
    if (queryType == "A+AAAA") {
        auto qa = new DnsLookup("A", name->currentText(), resolverUrl, this);
        auto qaaaa = new DnsLookup("AAAA", name->currentText(), resolverUrl, this);
        connect(qa, &DnsLookup::finished, [=](){
            qaaaa->lookup();
        });
        connect(qaaaa, &DnsLookup::finished, [=](){
            results->setText(qa->ErrorString() + qa->PrettyResults() + QString("\n") + qaaaa->ErrorString() + qaaaa->PrettyResults());
            qa->deleteLater();
            qaaaa->deleteLater();
        });
        qa->lookup();
        return;
    }

    auto q = new DnsLookup(queryType, name->currentText(), resolverUrl, this);
    connect(q, &DnsLookup::finished, [=](){
       results->setText(tr("Resolver: %1\nResponse time %2 ms\n\n").arg(resolverUrl).arg(static_cast<double>(q->responseTime()) / 1000000.0, 0, 'f', 3) + q->ErrorString() + q->PrettyResults());
       q->deleteLater();
    });
    q->lookup();
}
