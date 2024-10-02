#include "tdevicemodel.h"
#include "tpluginunitcontainer.h"

TDeviceModel::TDeviceModel(TCommon * unit, TPluginUnitContainer * parent, bool manual)
    : TProjectItem(parent->model(), parent), TPluginUnitModel(unit, parent, manual)
{

}

TDeviceModel::~TDeviceModel()
{
    if (!isInit())
        return;

    TDeviceModel::deInit();
}

int TDeviceModel::childrenCount() const
{
    return 0;
}

TProjectItem * TDeviceModel::child(int row) const
{
    return nullptr;
}
