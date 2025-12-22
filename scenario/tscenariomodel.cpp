#include "tscenario.h"
#include "tscenariomodel.h"
#include "tscenariocontainer.h"

TScenarioModel::TScenarioModel(TScenarioContainer * parent, TScenario * scenario)
    : TProjectUnitModel("scenario", parent, scenario)
{}

TScenario * TScenarioModel::scenario() const {
    return (TScenario *)m_unit;
}
