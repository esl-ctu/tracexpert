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

#include "tiodevicecontainer.h"

#include "../component/tcomponentmodel.h"

TIODeviceContainer::TIODeviceContainer(TComponentModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{
    m_typeName = "iodevices";
}

int TIODeviceContainer::count() const
{
    return m_IODevices.length();
}

TIODeviceModel * TIODeviceContainer::at(int index) const
{
    if (index >= 0 && index < count()) {
        return m_IODevices[index];
    }
    else {
        return nullptr;
    }
}

TIODeviceModel * TIODeviceContainer::getByName(const QString &name) const {
    for(TIODeviceModel * deviceModel : m_IODevices) {
        if(deviceModel->name() == name) {
            return deviceModel;
        }
    }

    return nullptr;
}

bool TIODeviceContainer::add(TIODeviceModel * unit)
{
    if (hasName(unit->name())) {
        return false;
    }

    beginInsertChild(m_IODevices.length());
    beginInsertRows(QModelIndex(), m_IODevices.length(), m_IODevices.length());
    m_IODevices.append(unit);
    endInsertRows();
    endInsertChild();

    return true;
}

bool TIODeviceContainer::remove(TIODeviceModel * unit)
{
    int index = m_IODevices.indexOf(unit);

    if (index < 0) {
        return false;
    }

    beginRemoveChild(index);
    beginRemoveRows(QModelIndex(), index, index);
    m_IODevices.remove(index);
    endRemoveRows();
    endRemoveChild();

    return true;
}

TIODeviceModel * TIODeviceContainer::hasName(QString name) const
{
    for (int i = 0; i < m_IODevices.length(); i++) {
        if (m_IODevices[i]->name() == name) {
            return m_IODevices[i];
        }
    }

    return nullptr;
}

QString TIODeviceContainer::name() const
{
    return tr("IO Devices");
}
