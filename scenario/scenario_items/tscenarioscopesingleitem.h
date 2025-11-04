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

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioScopeItem::setParams(params);

        TScopeModel * scopeModel = getDeviceModel();
        drawChannelOutputPorts(scopeModel);

        return paramsToReturn;
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        connect(m_deviceModel, &TScopeModel::runFailed, this, &TScenarioScopeItem::runFailed);
        connect(m_deviceModel, &TScopeModel::stopFailed, this, &TScenarioScopeItem::stopFailed);
        connect(m_deviceModel, &TScopeModel::downloadFailed, this, &TScenarioScopeItem::downloadFailed);

        connect(m_deviceModel, &TScopeModel::tracesDownloaded, this, &TScenarioScopeItem::tracesDownloaded);
        connect(m_deviceModel, &TScopeModel::tracesEmpty, this, &TScenarioScopeItem::tracesEmpty);
        connect(m_deviceModel, &TScopeModel::stopped, this, &TScenarioScopeItem::stopped);

        m_deviceModel->runSingle();
        log(QString(tr("[%1] Measurement started")).arg(m_deviceModel->name()));
    }
};

#endif // TSCENARIOSCOPESINGLEITEM_H
