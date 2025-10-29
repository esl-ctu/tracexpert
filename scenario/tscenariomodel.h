#ifndef TSCENARIOMODEL_H
#define TSCENARIOMODEL_H

#include <QObject>

#include "../project/tprojectitem.h"

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
class TScenarioModel : public QObject, public TProjectItem {
    Q_OBJECT

public:
    TScenarioModel(TScenarioContainer * parent);
    TScenarioModel(TScenario * scenario, TScenarioContainer * parent);

    ~TScenarioModel();

    TScenario * scenario() const;
    void setScenario(TScenario * scenario);

    // methods for TProjectItem - to be able to show Protocols in the Project view
    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    Status status() const override;

    bool toBeSaved() const override;
    QDomElement save(QDomDocument & document) const override;
    void load(QDomElement * element);

private:
    TScenario * m_scenario;
};



#endif // TSCENARIOMODEL_H
