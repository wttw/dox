#ifndef DOX_RESOLVERS_H
#define DOX_RESOLVERS_H

/*
 * Utilities to handle lists of resolvers
 */

#include <QString>
#include <QObject>

struct Resolver {
    QString name;
    QString url;
};

class QComboBox;

class Resolvers : public QObject {
    Q_OBJECT
public:
    Resolvers(QObject *parent = 0);
    QString loadFile(QString filename);
    QString saveFile(QString filename);

    QList<Resolver> list;

    void populateCombo(QComboBox *box);
signals:
    void resolversChanged();
public slots:
    void addResolver(QWidget *parent);
    void deleteResolver(int idx, QWidget *parent);
private:
    void save();
    void load();
};



#endif
