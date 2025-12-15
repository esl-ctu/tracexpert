#ifndef TSCENARIOMODEL_H
#define TSCENARIOMODEL_H

#include "../projectunit/tprojectunitmodel.h"

class TScenario;
class TScenarioContainer;

/*!
 * \brief The TScenarioModel class represents a model for a Scenario.
 *
 * The class represents a model for a Scenario.
 * It is a model for the Scenario view in the Project view.
 * It is also a model for the Scenario table view in the Scenario Manager.
 *
 */
class TScenarioModel : public TProjectUnitModel {

public:
    TScenarioModel(TScenarioContainer * parent);
    TScenarioModel(TScenario * scenario, TScenarioContainer * parent);

    TScenario * scenario() const;
};

#endif // TSCENARIOMODEL_H
