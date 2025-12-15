#ifndef TANALDEVICECONTAINER_H
#define TANALDEVICECONTAINER_H

#include "../tpluginunitcontainer.h"
#include "tanaldevicemodel.h"

class TComponentModel;

class TAnalDeviceContainer : public TPluginUnitContainer
{
    Q_OBJECT

public:
    explicit TAnalDeviceContainer(TComponentModel * parent = nullptr);

    int count() const override;
    TAnalDeviceModel * at(int index) const override;
    TAnalDeviceModel * getByName(const QString &name) const override;

    bool add(TAnalDeviceModel * unit);
    bool remove(TAnalDeviceModel * unit);

    TAnalDeviceModel * hasName(QString name) const;

    QString name() const override;

private:
    QList<TAnalDeviceModel *> m_analDevices;
};

#endif // TANALDEVICECONTAINER_H
