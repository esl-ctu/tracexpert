#include "tprotocol.h"
#include "tprotocolmodel.h"
#include "tprotocolcontainer.h"

TProtocolModel::TProtocolModel(TProtocolContainer * parent, TProtocol * protocol)
    : TProjectUnitModel("protocol", parent, protocol)
{}

TProtocol * TProtocolModel::protocol() const {
    return (TProtocol *)m_unit;
}
