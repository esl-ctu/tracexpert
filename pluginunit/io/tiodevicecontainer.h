#ifndef TIODEVICECONTAINER_H
#define TIODEVICECONTAINER_H

#include "../tpluginunitcontainer.h"
#include "tiodevicemodel.h"

class TComponentModel;

class TIODeviceContainer : public TPluginUnitContainer
{
    Q_OBJECT

public:
    explicit TIODeviceContainer(TComponentModel * parent);

    int count() const override;
    TIODeviceModel * at(int index) const override;
    TIODeviceModel * getByName(const QString &name) const override;

    bool add(TIODeviceModel * unit);
    bool remove(TIODeviceModel * unit);

    TIODeviceModel * hasName(QString name) const;

    QString name() const override;

private:
    QList<TIODeviceModel *> m_IODevices;
};

#endif // TIODEVICECONTAINER_H
