#ifndef TDEVICEMODEL_H
#define TDEVICEMODEL_H

#include "tpluginunitmodel.h"

class TDeviceModel : public TPluginUnitModel
{
    Q_OBJECT

public:
    explicit TDeviceModel(TCommon * unit, TPluginUnitContainer * parent, bool manual = false);
    virtual ~TDeviceModel();

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
};

#endif // TDEVICEMODEL_H
