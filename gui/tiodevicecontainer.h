#ifndef TIODEVICECONTAINER_H
#define TIODEVICECONTAINER_H

#include "tpluginunitcontainer.h"
#include "tiodevicemodel.h"

class TComponentModel;

class TIODeviceContainer : public TPluginUnitContainer
{
    Q_OBJECT

public:
    explicit TIODeviceContainer(TComponentModel * parent);

    int unitCount() const override;
    TIODeviceModel * unit(int index) const override;

    void addIODevice(TIODeviceModel * unit);

    QString name() const override;

private:
    QList<TIODeviceModel *> m_IODevices;
};

#endif // TIODEVICECONTAINER_H
