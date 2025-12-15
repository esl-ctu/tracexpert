#ifndef TSCENARIOANALDEVICEITEM_H
#define TSCENARIOANALDEVICEITEM_H

#include "tscenariocomponentitem.h"

/*!
 * \brief The TScenarioAnalDeviceItem class represents a block that represents an Analytic Device.
 *
 * The class represents a block that represents an Analytic Device.
 * It is a base class for Analytic Device Read and Write blocks, it cannot be used on its own.
 * In its basic form, is a block with one input flow port and two output flow ports.
 * The output flow ports are "done" and "error" - they are selected based on the success of the Analytic Device operation.
 * The Analytic Device can be selected in the configuration. It is selected from the list of available Analytic Devices in the selected Component.
 * The Analytic Device can be configured with pre-init and post-init parameters that can be set once,
 * before the scenario is launched, before the block is first executed, or each time the block is executed.
 *
 */
class TScenarioAnalDeviceItem : public TScenarioComponentItem<TAnalDeviceModel> {

public:
    TScenarioAnalDeviceItem(QString name, QString description)
        : TScenarioComponentItem<TAnalDeviceModel>("Analytic Device", name, description) { }

    bool supportsDirectExecution() const override {
        return false;
    }

    TScenarioItem * copy() const override {
        return new TScenarioAnalDeviceItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/analytic.png";
    }

    bool isCompatibleComponent(TComponentModel * componentModel) override {
        return componentModel->canAddAnalDevice() || componentModel->analDeviceCount() > 0;
    }

    int deviceCount(TComponentModel * componentModel) override {
        return componentModel->analDeviceCount();
    }

    TAnalDeviceModel * deviceByIndex(TComponentModel * componentModel, int index) override {
        return componentModel->analDevice(index);
    }

    TAnalDeviceModel * deviceByName(TComponentModel * componentModel, const QString & name) override {
        return componentModel->analDeviceContainer()->getByName(name);
    }
};

#endif // TSCENARIOANALDEVICEITEM_H
