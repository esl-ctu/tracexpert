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
// Vojtěch Miškovský (initial author)

#include "tpluginunitcontainer.h"

#include <QApplication>
#include <QStyle>

TPluginUnitContainer::TPluginUnitContainer(QObject * parent)
    : QAbstractTableModel(parent)
{

}

int TPluginUnitContainer::rowCount(const QModelIndex & parent) const
{
    return count();
}

int TPluginUnitContainer::columnCount(const QModelIndex & parent) const
{
    return 3;
}

QVariant TPluginUnitContainer::data(const QModelIndex & index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (index.column() == 1) {
            return at(index.row())->name();
        }
        else if (index.column() == 2) {
            return at(index.row())->info();
        }
    }
    else if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return TProjectItem::statusIcon(at(index.row())->status());
        }
    }
    return QVariant();
}

QVariant TPluginUnitContainer::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section == 0) {
                return tr("Status");
            }
            else if (section == 1) {
                return tr("Name");
            }
            else if (section == 2) {
                return tr("Description");
            }
        }
    }

    return QVariant();
}

int TPluginUnitContainer::childrenCount() const
{
    return count();
}

TProjectItem * TPluginUnitContainer::child(int row) const
{
    if (!count())
        return nullptr;

    return at(row);
}

TProjectItem::Status TPluginUnitContainer::status() const
{
    return None;
}
