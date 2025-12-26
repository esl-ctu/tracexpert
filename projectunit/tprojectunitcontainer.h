// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Adam Švehla (initial author)

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
