#ifndef TPROJECTUNITCONTAINER_H
#define TPROJECTUNITCONTAINER_H

#include "../project/tprojectitem.h"
#include <QAbstractTableModel>

class TProjectUnit;
class TProjectUnitModel;

class TProjectUnitContainer : public QAbstractTableModel, public TProjectItem {
    Q_OBJECT

public:
    TProjectUnitContainer(TProjectModel * parent);
    ~TProjectUnitContainer();

    void showManager();

    const QString & unitTypeName() const;

    TProjectUnitModel * getByName(const QString &name, bool *ok = nullptr) const;
    int getIndexByName(const QString &name, bool *ok = nullptr) const;

    int count() const;
    TProjectUnitModel * at(int index);

    bool add(TProjectUnitModel * itemModel);
    bool add(TProjectUnit * unit);
    bool update(int index, TProjectUnit * unit);
    bool remove(int index);

    // methods for QAbstractTableModel - to be able to show items in a table
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // methods for TProjectItem - to be able to show items in the Project view
    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    Status status() const override;

signals:
    void itemsUpdated();
    void showManagerRequested();
    void editorRequested(TProjectUnitModel * unitModel);

protected:
    using QAbstractTableModel::sort;
    void sort();

    QString m_unitTypeName = "unknown";

    QList<TProjectUnitModel *> m_unitModels;
};

#endif // TPROJECTUNITCONTAINER_H
