#include "tprotocolcontainer.h"

#include "tprotocolmodel.h"
#include "../project/tprojectmodel.h"

TProtocolContainer::TProtocolContainer(TProjectModel * parent)
    : TProjectUnitContainer(parent)
{
    m_typeName = "protocols";
    m_unitTypeName = "protocol";
}

QString TProtocolContainer::name() const
{
    return tr("Protocols");
}

