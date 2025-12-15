#include "tscenario.h"

#include "tscenariocontainer.h"
#include "../project/tprojectmodel.h"

TScenarioContainer::TScenarioContainer(TProjectModel * parent)
    : TProjectUnitContainer(parent)
{
    m_typeName = "scenarios";
    m_unitTypeName = "scenario";
}

QString TScenarioContainer::name() const
{
    return tr("Scenarios");
}
