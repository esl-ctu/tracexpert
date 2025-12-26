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
// Adam Švehla

#include "tcomponentcontainer.h"

#include "../../project/tprojectmodel.h"

TComponentContainer::TComponentContainer(TProjectModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{
    m_typeName = "components";
}

int TComponentContainer::count() const
{
    return m_components.length();
}

TComponentModel * TComponentContainer::at(int index) const
{
    if (index >= 0 && index < count()) {
        return m_components[index];
    }
    else {
        return nullptr;
    }
}

TComponentModel * TComponentContainer::getByName(const QString &name) const {
    for(TComponentModel * componentModel : m_components) {
        if(componentModel->name() == name) {
            return componentModel;
        }
    }

    return nullptr;
}

void TComponentContainer::add(TComponentModel * unit)
{
    unit->setParent(this);

    beginInsertChild(m_components.length());
    beginInsertRows(QModelIndex(), m_components.length(), m_components.length());
    m_components.append(unit);
    endInsertRows();
    endInsertChild();
}

QString TComponentContainer::name() const
{
    return tr("Components");
}
