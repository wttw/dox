#include "resolvers.h"

#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QCoreApplication>
#include <QFile>
#include <QSettings>
#include <QtDebug>

const char * const resolverGroup = "resolvers";
const char * const resolverName = "name";
const char * const resolverUrl = "url";

Resolvers::Resolvers(QObject *parent) : QObject(parent) {
    load();
    if (list.isEmpty()) {
        QString err = loadFile(":/defaultResolvers.json");
        Q_ASSERT(err.isEmpty());
        save();
    }
    emit resolversChanged();
}

// Load from app settings
void Resolvers::load() {
    list.clear();
    QSettings s;
    int size = s.beginReadArray(resolverGroup);
    for (int i = 0; i < size; ++i) {
        QJsonObject ob;
        s.setArrayIndex(i);
        list.append(Resolver{s.value(resolverName).toString(), s.value(resolverUrl).toString()});
    }
    s.endArray();
    emit resolversChanged();
}

// Save to app settings
void Resolvers::save() {
    QSettings s;
    s.beginWriteArray(resolverGroup, list.length());
    int i = 0;

    for (auto v : list) {
        s.setArrayIndex(i++);
        s.setValue(resolverName, v.name);
        s.setValue(resolverUrl, v.url);
    }
    s.endArray();
}

// Import from JSON
QString Resolvers::loadFile(QString filename) {
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return tr("Failed to open %1: %2").arg(filename, f.errorString());
    }

    QJsonParseError err;
    auto doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (doc.isNull()) {
        return tr("Failed to parse %1: %2").arg(filename, err.errorString());
    }

    QJsonValue res = doc.object().value("resolvers");
    if (res.isUndefined() || !res.isArray()) {
        return tr("Couldn't find resolvers array in %1").arg(filename);
    }

    QJsonArray resolvers = res.toArray();

    list.clear();

    for (auto v : resolvers) {
        QJsonObject resob = v.toObject();
        QString name = resob.value("name").toString();
        QString url = resob.value("url").toString();
        if (url.isEmpty() || name.isEmpty()) {
            qWarning() << "Failed to find name/url pair somewhere in" << filename;
        }
        list.append(Resolver{name, url});
    }
    save();
    emit resolversChanged();
    return QString();
}

// export to JSON
QString Resolvers::saveFile(QString filename) {
    QJsonArray resolvers;

    for (auto &r : list) {
        QJsonObject ob;
        ob.insert("name", r.name);
        ob.insert("url", r.url);
        resolvers.append(ob);
    }

    QJsonDocument doc(QJsonObject{{"resolvers", resolvers}});

    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return tr("Failed to create %1: %2").arg(filename, f.errorString());
    }
    f.write(doc.toJson(QJsonDocument::Indented));
    return QString();
}


void Resolvers::populateCombo(QComboBox *box) {
    QSettings s;
    box->clear();
    int i = 0;
    for (auto &r : list) {
        box->addItem(r.name, r.url);
        box->setItemData(box->currentIndex(), i, Qt::UserRole+1);
    }
}

void Resolvers::addResolver(QWidget *parent) {

}

void Resolvers::deleteResolver(int idx, QWidget *parent) {

}
