#ifndef TSCENARIOCONTAINER_H
#define TSCENARIOCONTAINER_H

#include <QAbstractTableModel>
#include "../projectunit/tprojectunitcontainer.h"

/*!
 * \brief The TScenarioContainer class represents a container for Scenarios.
 *
 * The class represents a container for TScenarioModel objects.
 * It is a model for the Scenarios view in the Project view.
 * It is also a model for the Scenarios table view in the Scenario Manager.
 */
class TScenarioContainer : public TProjectUnitContainer {

public:
    explicit TScenarioContainer(TProjectModel * parent);
    QString name() const override;
};

#endif // TSCENARIOCONTAINER_H
