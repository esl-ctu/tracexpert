#include "tiodevicecontainer.h"

#include "tcomponentmodel.h"

TIODeviceContainer::TIODeviceContainer(TComponentModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{

}

int TIODeviceContainer::unitCount() const
{
    return m_IODevices.length();
}

TIODeviceModel * TIODeviceContainer::unit(int index) const
{
    if (index >= 0 && index < unitCount()) {
        return m_IODevices[index];
    }
    else {
        return nullptr;
    }
}

void TIODeviceContainer::addIODevice(TIODeviceModel * unit)
{
    unit->setParent(this);

    beginInsertChild(m_IODevices.length());
    beginInsertRows(QModelIndex(), m_IODevices.length(), m_IODevices.length());
    m_IODevices.append(unit);
    endInsertRows();
    endInsertChild();
}

QString TIODeviceContainer::name() const
{
    return tr("IO Devices");
}
