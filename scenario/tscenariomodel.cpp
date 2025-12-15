#include "tscenario.h"
#include "tscenariomodel.h"
#include "tscenariocontainer.h"

TScenarioModel::TScenarioModel(TScenarioContainer * parent)
    : TProjectUnitModel("scenario", parent)
{}

TScenarioModel::TScenarioModel(TScenario * scenario, TScenarioContainer * parent)
    : TProjectUnitModel("scenario", scenario, parent)
{}

TScenario * TScenarioModel::scenario() const {
    return (TScenario *)m_unit;
}
