#include "survey_data.h"

#include <QJsonObject>
#include <QJsonArray>

QJsonValue SurveyTarget::toJson() const {
    return QJsonObject{
        {"name", name},
        {"url", url}
    };
}

bool SurveyTarget::fromJson(const QJsonValue& v) {
    auto ob = v.toObject();
    QJsonValue n = ob.value("name");
    QJsonValue u = ob.value("url");
    name = n.toString();
    url = u.toString();
    return n.isString() && u.isString();
}

QJsonValue SurveyResolver::toJson() const {
    return QJsonObject{
        {"name", name},
        {"url", url}
    };
}

bool SurveyResolver::fromJson(const QJsonValue& v) {
    auto ob = v.toObject();
    QJsonValue n = ob.value("name");
    QJsonValue u = ob.value("url");
    name = n.toString();
    url = u.toString();
    return n.isString() && u.isString();
}

SurveyResult::SurveyResult(QString t, QString r) : target(t), resolver(r), rttA(0), rttAAAA(0), rttFetch(0) {}

QJsonValue SurveyResult::toJson() const {
    QJsonArray addressList;
    for (auto &ip : addresses) {
        addressList.append(ip.toString());
    }

    return QJsonObject{
        {"target", target},
        {"resolver", resolver},
        {"addresses", addressList},
        {"rtt_a", static_cast<double>(rttA)},
        {"rtt_aaaa", static_cast<double>(rttAAAA)},
        {"rtt_content", static_cast<double>(rttFetch)},
        {"content_size", responseSize},
        {"content_error", responseError}
    };
}

bool SurveyResult::fromJson(const QJsonValue& v) {
    auto ob = v.toObject();
    QJsonArray addressList = ob.value("addresses").toArray();
    for (auto ad : addressList) {
        QHostAddress a(ad.toString());
        addresses.append(a);
    }
    target = ob.value("target").toString();
    resolver = ob.value("resolver").toString();
    rttA = ob.value("rtt_a").toDouble();
    rttAAAA = ob.value("rtt_aaaa").toDouble();
    rttFetch = ob.value("rtt_content").toDouble();
    responseSize = ob.value("content_size").toInt();
    responseError = ob.value("content_error").toInt();
    return true;
}

QJsonValue SurveyData::toJson() const {
    QJsonObject response;
    response.insert("meta", QJsonObject::fromVariantMap(meta));

    QJsonArray targetList;
    for (auto &v : targets) {
        targetList.append(v.toJson());
    }
    response.insert("targets", targetList);

    QJsonArray resolverList;
    for (auto &v : resolvers) {
        resolverList.append(v.toJson());
    }
    response.insert("resolvers", resolverList);

    QJsonArray resultList;
    for (auto &v : results) {
        resultList.append(v.toJson());
    }
    response.insert("results", resultList);

    return response;
}

bool SurveyData::fromJson(const QJsonValue &v) {
    clear();
    auto ob = v.toObject();
    meta = ob.value("meta").toVariant().toMap();
    auto targetList = ob.value("targets").toArray();
    for (auto tv : targetList) {
        SurveyTarget target;
        if (!target.fromJson(tv)) {
            return false;
        }
        targets.append(target);
        targetName.insert(target.url, target.name);
    }

    auto resolverList = ob.value("resolvers").toArray();
    for (auto rv : resolverList) {
        SurveyResolver resolver;
        if (!resolver.fromJson(rv)) {
            return false;
        }
        resolvers.append(resolver);
        resolverName.insert(resolver.url, resolver.name);
    }
}

void SurveyData::clear() {
    meta.clear();
    targets.clear();
    resolvers.clear();
    results.clear();
    resolverName.clear();
    targetName.clear();
}

