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

#include "tscopecontainer.h"

#include "../component/tcomponentmodel.h"

TScopeContainer::TScopeContainer(TComponentModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{
    m_typeName = "scopes";
}

int TScopeContainer::count() const
{
    return m_scopes.length();
}

TScopeModel * TScopeContainer::at(int index) const
{
    if (index >= 0 && index < count()) {
        return m_scopes[index];
    }
    else {
        return nullptr;
    }
}

TScopeModel * TScopeContainer::getByName(const QString &name) const {
    for(TScopeModel * scopeModel : m_scopes) {
        if(scopeModel->name() == name) {
            return scopeModel;
        }
    }

    return nullptr;
}

void TScopeContainer::add(TScopeModel * unit)
{
    unit->setParent(this);

    beginInsertChild(m_scopes.length());
    beginInsertRows(QModelIndex(), m_scopes.length(), m_scopes.length());
    m_scopes.append(unit);
    endInsertRows();
    endInsertChild();
}

bool TScopeContainer::remove(TScopeModel * unit)
{
    int index = m_scopes.indexOf(unit);

    if (index < 0) {
        return false;
    }

    beginRemoveChild(index);
    beginRemoveRows(QModelIndex(), index, index);
    m_scopes.remove(index);
    endRemoveRows();
    endRemoveChild();

    return true;
}

TScopeModel * TScopeContainer::hasName(QString name) const
{
    for (int i = 0; i < m_scopes.length(); i++) {
        if (m_scopes[i]->name() == name) {
            return m_scopes[i];
        }
    }

    return nullptr;
}

QString TScopeContainer::name() const
{
    return tr("Oscilloscopes");
}
