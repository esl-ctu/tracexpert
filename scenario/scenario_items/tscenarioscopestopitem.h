#ifndef TSCENARIOSCOPESTOPITEM_H
#define TSCENARIOSCOPESTOPITEM_H

#include "../tscenarioitem.h"
#include "tscenarioscopeitem.h"

class TScenarioScopeStartItem;

class TScenarioScopeStopItem : public TScenarioScopeItem {

public:
    enum { TItemClass = 63 };
    int itemClass() const override { return TItemClass; }

    TScenarioScopeStopItem() :
        TScenarioScopeItem(
              tr("Oscilloscope: download data"),
              tr("This block obtains data from the previously started measurement.")
        )
    {
        addConnectionInputPort("startConnection", "", tr("Connect a \"start measurement\" block to this port."));
        initializeDataOutputPorts();
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeStopItem(*this);
    }

    bool shouldUpdateParams(TConfigParam newParams) override;
    void updateParams(bool paramValuesChanged) override;

    bool prepare() override;
    void execute(const QHash<TScenarioItemPort *, QByteArray> & inputData) override;

    void clearOutputData() {
        m_outputData.clear();
    }

protected:
    TScenarioScopeStartItem * m_startItem;

};

#endif // TSCENARIOSCOPESTOPITEM_H
