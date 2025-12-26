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

#ifndef TSCENARIOSCOPESINGLEITEM_H
#define TSCENARIOSCOPESINGLEITEM_H

#include "../tscenarioitem.h"
#include "tscenarioscopeitem.h"

class TScenarioScopeSingleItem : public TScenarioScopeItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioScopeSingleItem;
    }

    TScenarioScopeSingleItem() :
        TScenarioScopeItem(
              tr("Oscilloscope: single capture"),
              tr("This block performs a single capture using the selected Oscilloscope.")
        )
    {
        initializeDataOutputPorts();
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeSingleItem(*this);
    }

    bool supportsDirectExecution() const override {
        return false;
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioScopeItem::setParams(params);

        TScopeModel * scopeModel = getDeviceModel();
        drawChannelOutputPorts(scopeModel);

        return paramsToReturn;
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        clearOutputData();

        connect(m_deviceModel, &TScopeModel::runFailed, this, &TScenarioScopeItem::runFailed);
        connect(m_deviceModel, &TScopeModel::stopFailed, this, &TScenarioScopeItem::stopFailed);
        connect(m_deviceModel, &TScopeModel::downloadFailed, this, &TScenarioScopeItem::downloadFailed);

        connect(m_deviceModel, &TScopeModel::tracesDownloaded, this, &TScenarioScopeItem::tracesDownloaded);
        connect(m_deviceModel, &TScopeModel::tracesEmpty, this, &TScenarioScopeItem::tracesEmpty);
        connect(m_deviceModel, &TScopeModel::stopped, this, &TScenarioScopeItem::stopped);

        m_deviceModel->runSingle();
        log(QString(tr("[%1] Measurement started")).arg(m_deviceModel->name()));
    }

    void runFailed() override {
        TScenarioScopeItem::runFailed();
        emit executionFinished();
    }

    void stopFailed() override {
        TScenarioScopeItem::stopFailed();
        emit executionFinished();
    }

    void downloadFailed() override {
        TScenarioScopeItem::downloadFailed();
        emit executionFinished();
    }

    void tracesEmpty() override {
        TScenarioScopeItem::tracesEmpty();
        emit executionFinished();
    }

    void stopped() override {
        TScenarioScopeItem::stopped();
        emit executionFinished(m_outputData);
    }
};

#endif // TSCENARIOSCOPESINGLEITEM_H
