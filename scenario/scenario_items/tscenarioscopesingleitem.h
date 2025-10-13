#ifndef TSCENARIOSCOPESINGLEITEM_H
#define TSCENARIOSCOPESINGLEITEM_H

#include "../tscenarioitem.h"
#include "tscenarioscopeitem.h"

class TScenarioScopeSingleItem : public TScenarioScopeItem {

public:
    enum { TItemClass = 61 };
    int itemClass() const override { return TItemClass; }

    TScenarioScopeSingleItem() :
        TScenarioScopeItem(
              tr("Oscilloscope: single capture"),
              tr("This block performs a single capture using the selected Oscilloscope.")
        )
    {        
        initializeDataOutputPorts();
        initializeConfigParams();
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeSingleItem(*this);
    }

    void execute(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        connect(m_scopeModel, &TScopeModel::runFailed, this, &TScenarioScopeItem::runFailed);
        connect(m_scopeModel, &TScopeModel::stopFailed, this, &TScenarioScopeItem::stopFailed);
        connect(m_scopeModel, &TScopeModel::downloadFailed, this, &TScenarioScopeItem::downloadFailed);

        connect(m_scopeModel, &TScopeModel::tracesDownloaded, this, &TScenarioScopeItem::tracesDownloaded);
        connect(m_scopeModel, &TScopeModel::tracesEmpty, this, &TScenarioScopeItem::tracesEmpty);
        connect(m_scopeModel, &TScopeModel::stopped, this, &TScenarioScopeItem::stopped);

        m_scopeModel->runSingle();
        log(QString(tr("[%1] Measurement started")).arg(m_scopeModel->name()));
    }
};

#endif // TSCENARIOSCOPESINGLEITEM_H
