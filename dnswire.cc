#include "dnswire.h"

#include "ldns/ldns.h"

// TODO(steve): less hostile logging / error handling
#include <QtEndian>
#include <QtDebug>


QByteArray WireRequest(ushort type, const QString &name) {
    // TODO(steve): Handle IDN
    QByteArray asAscii = name.toUtf8();
    ldns_rdf *qname = ldns_dname_new_frm_str(asAscii.constData());
    if (qname == nullptr) {
        qWarning() << "Failed to convert to qname " << name;
        return QByteArray();
    }
    ldns_resolver * res = ldns_resolver_new();
    Q_ASSERT(res != nullptr);
    ldns_pkt *qpkt;
    auto status = ldns_resolver_prepare_query_pkt(&qpkt, res, qname, static_cast<ldns_rr_type>(type), LDNS_RR_CLASS_IN, LDNS_RD);
    if (status != LDNS_STATUS_OK) {
        qWarning() << "Failed to prepare query packet for" << name << ": " << ldns_get_errorstr_by_id(status);
        return QByteArray();
    }

//    qDebug() << "Query packet" << ldns_pkt2str(qpkt);

    uint8_t *wire = NULL;
    size_t size;
    status = ldns_pkt2wire(&wire, qpkt, &size);

    if (size == 0) {
        return QByteArray();
    }

    QByteArray ret(reinterpret_cast<char*>(wire), size);
    ldns_pkt_free(qpkt);
    free(wire);
    return ret;
}

QString WireToPretty(const QByteArray &wire) {
    ldns_pkt * rpkt;
    ldns_status status = ldns_wire2pkt(&rpkt, reinterpret_cast<const uint8_t*>(wire.constData()), wire.length());
    if (status != LDNS_STATUS_OK) {
        qWarning() << "Failed to parse response: " << ldns_get_errorstr_by_id(status);
        return QString();
    }
    QString ret;
    char *str = ldns_pkt2str(rpkt);
    if (str == nullptr) {
        qWarning() << "Failed to stringify response";
    } else {
        ret = QString::fromUtf8(str);
    }

    ldns_pkt_free(rpkt);

    return ret;
}

QList<QHostAddress> WireToAddresses(const QByteArray &wire) {
    if (wire.isEmpty()) {
        return QList<QHostAddress>();
    }
    ldns_pkt * rpkt;
    ldns_status status = ldns_wire2pkt(&rpkt, reinterpret_cast<const uint8_t*>(wire.constData()), wire.length());
    if (status != LDNS_STATUS_OK) {
        qWarning() << "Failed to parse response: " << ldns_get_errorstr_by_id(status);
        return QList<QHostAddress>();
    }

    QList<QHostAddress> ret;
    auto arrs = ldns_pkt_rr_list_by_type(rpkt, LDNS_RR_TYPE_A, LDNS_SECTION_ANSWER);
    for (size_t i = 0; i < ldns_rr_list_rr_count(arrs); ++i) {
        ldns_rr *rr = ldns_rr_list_rr(arrs, i);
        Q_ASSERT(ldns_rr_get_type(rr) == LDNS_RR_TYPE_A);
        Q_ASSERT(ldns_rr_rd_count(rr) == 1);
        auto rdf = ldns_rr_rdf(rr, 0);
        Q_ASSERT(ldns_rdf_get_type(rdf) == LDNS_RDF_TYPE_A);
        Q_ASSERT(ldns_rdf_size(rdf) == 4);
        quint32 addr = qFromBigEndian<quint32>(ldns_rdf_data(rdf));
        ret.append(QHostAddress(addr));
    }

    auto aaaarrs =  ldns_pkt_rr_list_by_type(rpkt, LDNS_RR_TYPE_AAAA, LDNS_SECTION_ANSWER);
    for (size_t i = 0; i < ldns_rr_list_rr_count(aaaarrs); ++i) {
        ldns_rr *rr = ldns_rr_list_rr(aaaarrs, i);
        Q_ASSERT(ldns_rr_get_type(rr) == LDNS_RR_TYPE_AAAA);
        Q_ASSERT(ldns_rr_rd_count(rr) == 1);
        auto rdf = ldns_rr_rdf(rr, 0);
        Q_ASSERT(ldns_rdf_get_type(rdf) == LDNS_RDF_TYPE_AAAA);
        Q_ASSERT(ldns_rdf_size(rdf) == 16);
        ret.append(QHostAddress(ldns_rdf_data(rdf)));
    }

    ldns_pkt_free(rpkt);
    return ret;
}
