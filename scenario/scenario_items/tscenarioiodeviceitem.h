#ifndef TSCENARIOIODEVICEITEM_H
#define TSCENARIOIODEVICEITEM_H

#include "tscenariocomponentitem.h"

/*!
 * \brief The TScenarioIODeviceItem class represents a block that represents an IO Device.
 *
 * The class represents a block that represents an IO Device.
 * It is a base class for IO Device Read and Write blocks, it cannot be used on its own.
 * In its basic form, is a block with one input flow port and two output flow ports.
 * The output flow ports are "done" and "error" - they are selected based on the success of the IO Device operation.
 * The IO Device can be selected in the configuration. It is selected from the list of available IO Devices in the selected Component.
 * The IO Device can be configured with pre-init and post-init parameters that can be set once,
 * before the scenario is launched, before the block is first executed, or each time the block is executed.
 *
 */
class TScenarioIODeviceItem : public TScenarioComponentItem<TIODeviceModel> {

public:
    TScenarioIODeviceItem (QString name, QString description)
        : TScenarioComponentItem<TIODeviceModel>("IO Device", name, description) { }

    bool supportsDirectExecution() const override {
        return false;
    }

    TScenarioItem * copy() const override {
        return new TScenarioIODeviceItem (*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/file.png";
    }

    bool isCompatibleComponent(TComponentModel * componentModel) override {
        return componentModel->canAddIODevice() || componentModel->IODeviceCount() > 0;
    }

    int deviceCount(TComponentModel * componentModel) override {
        return componentModel->IODeviceCount();
    }

    TIODeviceModel * deviceByIndex(TComponentModel * componentModel, int index) override {
        return componentModel->IODevice(index);
    }

    TIODeviceModel * deviceByName(TComponentModel * componentModel, const QString & name) override {
        return componentModel->IODeviceContainer()->getByName(name);
    }
};

#endif // TSCENARIOIODEVICEITEM_H
