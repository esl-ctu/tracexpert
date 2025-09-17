#ifndef TSCENARIOSCOPESTARTITEM_H
#define TSCENARIOSCOPESTARTITEM_H

#include "../tscenarioitem.h"
#include "tscenarioscopeitem.h"

class TScenarioScopeStopItem;

class TScenarioScopeStartItem : public TScenarioScopeItem {

public:
    enum { TItemClass = 62 };
    int itemClass() const override { return TItemClass; }

    TScenarioScopeStartItem() :
        TScenarioScopeItem(
              tr("Oscilloscope: start measurement"),
              tr("This block starts measurement for the selected Oscilloscope.")
        )
    {
        addConnectionOutputPort("stopConnection", "", tr("Connect a \"download data\" block to this port."));
        initializeConfigParams();
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeStartItem(*this);
    }

    bool supportsImmediateExecution() const override {
        return true;
    }

    bool prepare() override;

    QHash<TScenarioItemPort *, QByteArray> executeImmediate(const QHash<TScenarioItemPort *, QByteArray> & inputData) override;

protected:
    TScenarioScopeStopItem * m_stopItem;

};

#endif // TSCENARIOSCOPESTARTITEM_H
