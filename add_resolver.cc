#include "add_resolver.h"

#include <QComboBox>
#include <QStackedWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QLabel>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QUrl>
#include <QWidget>
#include <QPushButton>

AddResolver::AddResolver(QWidget *parent) : QDialog(parent) {
    auto layout = new QVBoxLayout;
    auto form = new QFormLayout;
    layout->addLayout(form);
    name = new QLineEdit;
    form->addRow(tr("Description"), name);
    chooser = new QComboBox;
    form->addRow(tr("Flavour"), chooser);
    pages = new QStackedWidget;
    layout->addWidget(pages, 1);

    connect(chooser, qOverload<int>(&QComboBox::currentIndexChanged), pages, &QStackedWidget::setCurrentIndex);
    connect(chooser, qOverload<int>(&QComboBox::currentIndexChanged), this, &AddResolver::somethingChanged);

    bbox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    connect(bbox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(bbox);

    setLayout(layout);
    somethingChanged();
}

void AddResolver::somethingChanged() {
    auto page = qobject_cast<SomeResolverCfg*>(pages->currentWidget());
    Q_ASSERT(page);
    bbox->button(QDialogButtonBox::Save)->setDisabled(page->url().isEmpty());
}

void AddResolver::addPage(QString name, SomeResolverCfg *page) {
    chooser->addItem(name);
    pages->addWidget(page);
}

SomeResolverCfg::SomeResolverCfg(AddResolver *parent) : QWidget(parent) {

}

SystemResolverCfg::SystemResolverCfg(AddResolver *parent) : SomeResolverCfg(parent) {
    auto layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("This resolver will use the default DNS resolver for the application is (specifically Qt's QDnsLookup class)")));
    setLayout(layout);
}

QString SystemResolverCfg::url() const {
    return "system";
}

UdpResolverCfg::UdpResolverCfg(AddResolver *parent) : SomeResolverCfg(parent) {
    auto layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("This is a traditional RFC 1035 DNS resolver using UDP on port 53")));
    auto form = new QFormLayout;
    layout->addLayout(form);
    server = new QLineEdit;
    server->setPlaceholderText(tr("e.g. 8.8.8.8 or dns.google"));
    server->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9.:-]+$"), server));
    connect(server, &QLineEdit::textChanged, parent, &AddResolver::somethingChanged);
    form->addRow(tr("Server"), server);
    port = new QLineEdit;
    port->setText("53");
    port->setValidator(new QIntValidator(1, 65535, port));
    connect(port, &QLineEdit::textChanged, parent, &AddResolver::somethingChanged);
    form->addRow(tr("Port"), port);
    layout->addStretch();
}

QString UdpResolverCfg::url() const {
    if (server->hasAcceptableInput() && port->hasAcceptableInput()) {
        return "udp:" + server->text() + ":" + port->text();
    }
    return QString();
}

DotResolverCfg::DotResolverCfg(AddResolver *parent) : SomeResolverCfg(parent) {
    auto layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("This is an RFC 7858 resolver over TLS")));
    auto form = new QFormLayout;
    layout->addLayout(form);
    server = new QLineEdit;
    server->setPlaceholderText(tr("e.g. 8.8.8.8 or dns.google"));
    server->setValidator(new QRegularExpressionValidator(QRegularExpression("^[a-zA-Z0-9.:-]+$"), server));
    connect(server, &QLineEdit::textChanged, parent, &AddResolver::somethingChanged);
    form->addRow(tr("Server"), server);
    port = new QLineEdit;
    port->setText("853");
    port->setValidator(new QIntValidator(1, 65535, port));
    connect(port, &QLineEdit::textChanged, parent, &AddResolver::somethingChanged);
    form->addRow(tr("Port"), port);
    layout->addStretch();
}

QString DotResolverCfg::url() const {
    if (server->hasAcceptableInput() && port->hasAcceptableInput()) {
        return "dot:" + server->text() + ":" + port->text();
    }
    return QString();
}

PostResolverCfg::PostResolverCfg(AddResolver *parent) : SomeResolverCfg(parent) {
    auto layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("This is an RFC 8484 wireformat over https POST resolver")));
    auto form = new QFormLayout;
    layout->addLayout(form);
    server = new QLineEdit;
    server->setPlaceholderText(tr("e.g. https://cloudflare-dns.com/dns-query"));
    connect(server, &QLineEdit::textChanged, parent, &AddResolver::somethingChanged);
    layout->addStretch();
}

QString PostResolverCfg::url() const {
    QUrl u(server->text());
    if (u.isValid() && server->text().startsWith("https:")) {
        return server->text();
    }
    return QString();
}

GetResolverCfg::GetResolverCfg(AddResolver *parent) : SomeResolverCfg(parent) {
    auto layout = new QVBoxLayout;
    layout->addWidget(new QLabel(tr("This is an RFC 8484 wireformat over https GET resolver")));
    auto form = new QFormLayout;
    layout->addLayout(form);
    server = new QLineEdit;
    server->setPlaceholderText(tr("e.g. https://cloudflare-dns.com/dns-query{?dns}"));
    connect(server, &QLineEdit::textChanged, parent, &AddResolver::somethingChanged);
    layout->addStretch();
}

QString GetResolverCfg::url() const {
    QUrl u(server->text());
    if (u.isValid() && server->text().startsWith("https:") && server->text().contains("{") && server->text().endsWith("}")) {
        return server->text();
    }
    return QString();
}

