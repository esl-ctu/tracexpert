#include "tprotocolmodel.h"
#include "tprotocolcontainer.h"

TProtocolModel::TProtocolModel(TProtocol protocol, TProtocolContainer * parent) : QObject(parent), TProjectItem(parent->model(), parent), m_protocol(protocol) { }

const TProtocol & TProtocolModel::protocol() const {
    return m_protocol;
}

void TProtocolModel::setProtocol(const TProtocol & protocol) {
    m_protocol = protocol;
}

int TProtocolModel::childrenCount() const
{
    return 0;
}

TProjectItem * TProtocolModel::child(int row) const
{
    return nullptr;
}

QString TProtocolModel::name() const
{
    return m_protocol.getName();
}

QVariant TProtocolModel::status() const
{
    return QString("");
}
