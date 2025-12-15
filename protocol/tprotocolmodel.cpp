#include "tprotocol.h"
#include "tprotocolmodel.h"
#include "tprotocolcontainer.h"

TProtocolModel::TProtocolModel(TProtocolContainer * parent)
    : TProjectUnitModel("protocol", parent)
{}

TProtocolModel::TProtocolModel(TProtocol * protocol, TProtocolContainer * parent)
    : TProjectUnitModel("protocol", protocol, parent)
{}

TProtocol * TProtocolModel::protocol() const {
    return (TProtocol *)m_unit;
}
