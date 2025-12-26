// COPYRIGHT HEADER BEGIN
// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Adam Švehla (initial author)
// COPYRIGHT HEADER END

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
