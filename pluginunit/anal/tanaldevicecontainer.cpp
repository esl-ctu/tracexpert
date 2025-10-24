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
