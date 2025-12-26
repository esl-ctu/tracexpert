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

#ifndef TSCENARIOSCOPESTARTITEM_H
#define TSCENARIOSCOPESTARTITEM_H

#include "../tscenarioitem.h"
#include "tscenarioscopeitem.h"

class TScenarioScopeStopItem;

class TScenarioScopeStartItem : public TScenarioScopeItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioScopeStartItem;
    }

    TScenarioScopeStartItem() :
        TScenarioScopeItem(
              tr("Oscilloscope: start measurement"),
              tr("This block starts measurement for the selected Oscilloscope.")
        )
    {
        addConnectionOutputPort("stopConnection", "", tr("Connect a \"download data\" block to this port."));
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeStartItem(*this);
    }

    bool supportsDirectExecution() const override {
        return true;
    }

    TScenarioScopeItem * getConnectedItem() {
        if(!this->getItemPortByName("stopConnection")->hasConnectedPort()) {
            return nullptr;
        }

        QSet<TScenarioItemPort*> connectedItemPorts = this->getItemPortByName("stopConnection")->getConnectedPorts();
        TScenarioItem * connectedItem = (*connectedItemPorts.begin())->getParentItem();

        if(connectedItem->itemClass() != TItemClass::TScenarioScopeStopItem) {
            return nullptr;
        }

        return (TScenarioScopeItem *)connectedItem;
    }

    bool prepare() override {
        if(!TScenarioComponentItem::prepare()) {
            return false;
        }

        m_stopItem = getConnectedItem();
        if(!m_stopItem) {
            setState(TState::TError, tr("Failed to get \"download data\" item, is it connected?"));
            return false;
        }

        return true;
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioScopeItem::setParams(params);

        TScopeModel * scopeModel = getDeviceModel();

        m_stopItem = getConnectedItem();
        if(!m_stopItem) {
            setState(TState::TWarning, tr("Remember to connect the \"download data\" item. "
                                       "It is necessary to confirm this block's parameters again "
                                       "to update the number of output data (channel) ports."));
        }
        else {
            m_stopItem->drawChannelOutputPorts(scopeModel);
        }

        return paramsToReturn;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        m_stopItem->clearOutputData();

        // the following events are connected to the connected block instance
        connect(m_deviceModel, &TScopeModel::runFailed, m_stopItem, &TScenarioScopeItem::runFailed);
        connect(m_deviceModel, &TScopeModel::stopFailed, m_stopItem, &TScenarioScopeItem::stopFailed);
        connect(m_deviceModel, &TScopeModel::downloadFailed, m_stopItem, &TScenarioScopeItem::downloadFailed);

        connect(m_deviceModel, &TScopeModel::tracesDownloaded, m_stopItem, &TScenarioScopeItem::tracesDownloaded);
        connect(m_deviceModel, &TScopeModel::tracesEmpty, m_stopItem, &TScenarioScopeItem::tracesEmpty);
        connect(m_deviceModel, &TScopeModel::stopped, m_stopItem, &TScenarioScopeItem::stopped);

        m_deviceModel->runSingle();
        log(QString(tr("[%1] Measurement started")).arg(m_deviceModel->name()));

        return {};
    }

protected:
    TScenarioScopeItem * m_stopItem = nullptr;

};

#endif // TSCENARIOSCOPESTARTITEM_H
