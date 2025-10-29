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
