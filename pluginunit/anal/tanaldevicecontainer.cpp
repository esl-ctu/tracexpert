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

#include "tanaldevicecontainer.h"

#include "../component/tcomponentmodel.h"

TAnalDeviceContainer::TAnalDeviceContainer(TComponentModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{
    m_typeName = "analdevices";
}

int TAnalDeviceContainer::count() const
{
    return m_analDevices.length();
}

TAnalDeviceModel * TAnalDeviceContainer::at(int index) const
{
    if (index >= 0 && index < count()) {
        return m_analDevices[index];
    }
    else {
        return nullptr;
    }
}

TAnalDeviceModel * TAnalDeviceContainer::getByName(const QString &name) const {
    for(TAnalDeviceModel * deviceModel : m_analDevices) {
        if(deviceModel->name() == name) {
            return deviceModel;
        }
    }

    return nullptr;
}

bool TAnalDeviceContainer::add(TAnalDeviceModel * unit)
{
    if (hasName(unit->name())) {
        return false;
    }

    beginInsertChild(m_analDevices.length());
    beginInsertRows(QModelIndex(), m_analDevices.length(), m_analDevices.length());
    m_analDevices.append(unit);
    endInsertRows();
    endInsertChild();

    return true;
}

bool TAnalDeviceContainer::remove(TAnalDeviceModel * unit)
{
    int index = m_analDevices.indexOf(unit);

    if (index < 0) {
        return false;
    }

    beginRemoveChild(index);
    beginRemoveRows(QModelIndex(), index, index);
    m_analDevices.remove(index);
    endRemoveRows();
    endRemoveChild();

    return true;
}

TAnalDeviceModel * TAnalDeviceContainer::hasName(QString name) const
{
    for (int i = 0; i < m_analDevices.length(); i++) {
        if (m_analDevices[i]->name() == name) {
            return m_analDevices[i];
        }
    }

    return nullptr;
}

QString TAnalDeviceContainer::name() const
{
    return tr("Analytical Devices");
}
