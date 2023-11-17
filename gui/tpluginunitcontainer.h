#ifndef TPLUGINUNITCONTAINER_H
#define TPLUGINUNITCONTAINER_H

#include <QAbstractTableModel>

#include "tpluginunitmodel.h"
#include "tprojectitem.h"

class TPluginUnitContainer : public QAbstractTableModel, public virtual TProjectItem
{
    Q_OBJECT

public:
    explicit TPluginUnitContainer(QObject * parent = nullptr);

    virtual int count() const = 0;
    virtual TPluginUnitModel * at(int index) const = 0;

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QVariant status() const override;
};

#endif // TPLUGINUNITCONTAINER_H
