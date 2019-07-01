#ifndef DOX_ADD_RESOLVER_H
#define DOX_ADD_RESOLVER_H

#include <QDialog>

class QComboBox;
class QStackedWidget;
class QLineEdit;
class QWidget;
class QDialogButtonBox;

class SomeResolverCfg;

class AddResolver : public QDialog {
  Q_OBJECT
public:
    AddResolver(QWidget *parent = 0);
public slots:
    void somethingChanged();
private:
    QLineEdit *name;
    QComboBox *chooser;
    QStackedWidget *pages;
    QDialogButtonBox *bbox;
    void addPage(QString name, SomeResolverCfg *page);
};

class SomeResolverCfg : public QWidget {
    Q_OBJECT
public:
    SomeResolverCfg(AddResolver *parent);
    virtual QString url() const = 0;
};

class SystemResolverCfg : public SomeResolverCfg {
    Q_OBJECT
public:
    SystemResolverCfg(AddResolver *parent = 0);
    QString url() const;
};

class UdpResolverCfg : public SomeResolverCfg {
    Q_OBJECT
public:
    UdpResolverCfg(AddResolver *parent = 0);
    QString url() const;
private:
    QLineEdit *server;
    QLineEdit *port;
};

class DotResolverCfg : public SomeResolverCfg {
    Q_OBJECT
public:
    DotResolverCfg(AddResolver *parent = 0);
    QString url() const;
private:
    QLineEdit *server;
    QLineEdit *port;
};

class PostResolverCfg : public SomeResolverCfg {
    Q_OBJECT
public:
    PostResolverCfg(AddResolver *parent = 0);
    QString url() const;
private:
    QLineEdit *server;
};

class GetResolverCfg : public SomeResolverCfg {
    Q_OBJECT
public:
    GetResolverCfg(AddResolver *parent = 0);
    QString url() const;
private:
    QLineEdit *server;
};

#endif
