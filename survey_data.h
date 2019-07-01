#ifndef DOX_SURVEY_DATA_H
#define DOX_SURVEY_DATA_H

#include <QString>
#include <QMap>
#include <QJsonValue>
#include <QHostAddress>

class SurveyTarget {
  public:
    QString name;
    QString url;
    QJsonValue toJson() const;
    bool fromJson(const QJsonValue& v);
};

class SurveyResolver {
public:
    QString name;
    QString url;
    QJsonValue toJson() const;
    bool fromJson(const QJsonValue& v);
};

class SurveyResult {
public:
    SurveyResult(QString t, QString r);
    QString target;
    QString resolver;
    QList<QHostAddress> addresses;
    qint64 rttA;
    qint64 rttAAAA;
    qint64 rttFetch;
    int responseSize;
    int responseError;
    QJsonValue toJson() const;
    bool fromJson(const QJsonValue& v);
};

class SurveyData {
public:
    QVariantMap meta;
    QList<SurveyTarget> targets;
    QList<SurveyResolver> resolvers;
    QList<SurveyResult> results;
    QMap<QString,QString> resolverName;
    QMap<QString,QString> targetName;
    QJsonValue toJson() const;
    bool fromJson(const QJsonValue& v);
    void clear();
};

#endif
