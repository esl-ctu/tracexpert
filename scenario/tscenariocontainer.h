#ifndef TSCENARIOCONTAINER_H
#define TSCENARIOCONTAINER_H

#include <QAbstractTableModel>
#include "tscenariomodel.h"
#include "../project/tprojectitem.h"

/*!
 * \brief The TScenarioContainer class represents a container for Scenarios.
 *
 * The class represents a container for TScenarioModel objects.
 * It is a model for the Scenarios view in the Project view.
 * It is also a model for the Scenarios table view in the Scenario Manager.
 */
class TScenarioContainer : public QAbstractTableModel, public TProjectItem {
    Q_OBJECT

public:
    explicit TScenarioContainer(TProjectModel * parent);
    ~TScenarioContainer();

    TScenarioModel * getByName(const QString &name, bool *ok = nullptr) const;
    int getIndexByName(const QString &name, bool *ok = nullptr) const;

    int count() const;
    TScenarioModel * at(int index);

    bool add(TScenarioModel * scenario);
    bool add(TScenario * scenario);
    bool update(int index, TScenario * scenario);
    bool remove(int index);

    // methods for QAbstractTableModel - to be able to show Scenarios in a table
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // methods for TProjectItem - to be able to show Scenarios in the Project view
    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    Status status() const override;

signals:
    void scenariosUpdated();

private:
    using QAbstractTableModel::sort;
    void sort();

    QList<TScenarioModel *> m_scenarioModels;
};


#endif // TSCENARIOCONTAINER_H
