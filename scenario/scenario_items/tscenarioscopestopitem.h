#ifndef TSCENARIOSCOPESTOPITEM_H
#define TSCENARIOSCOPESTOPITEM_H

#include "../tscenarioitem.h"
#include "tscenarioscopeitem.h"


class TScenarioScopeStartItem;

class TScenarioScopeStopItem : public TScenarioScopeItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioScopeStopItem;
    }

    TScenarioScopeStopItem() :
        TScenarioScopeItem(
              tr("Oscilloscope: download data"),
              tr("This block obtains data from the previously started measurement.")
        )
    {
        addConnectionInputPort("startConnection", "", tr("Connect a \"start measurement\" block to this port."));
        initializeDataOutputPorts();

        // reset config params, subtitle and state
        // as this block doesn't require configuration
        m_params = TConfigParam();
        m_subtitle = "";
        resetState();
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeStopItem(*this);
    }

    bool supportsDirectExecution() const override {
        return false;
    }

    void updateParams(bool paramValuesChanged) override { }

    TScenarioScopeItem * getConnectedItem() {
        if(!this->getItemPortByName("startConnection")->hasConnectedPort()) {
            return nullptr;
        }

        QSet<TScenarioItemPort*> connectedItemPorts = this->getItemPortByName("startConnection")->getConnectedPorts();
        TScenarioItem * connectedItem = (*connectedItemPorts.begin())->getParentItem();

        if(connectedItem->itemClass() != TItemClass::TScenarioScopeStartItem) {
            return nullptr;
        }

        return (TScenarioScopeItem *)connectedItem;
    }

    bool prepare() override {
        resetState();

        TScenarioScopeItem * startItem = getConnectedItem();
        if(!startItem) {
            setState(TState::TError, tr("Failed to get \"start measurement\" item, is it connected?"));
            return false;
        }

        m_deviceModel = startItem->getDeviceModel();

        m_deviceSignalReceived = false;
        m_waitingForExecutionFinish = false;

        return true;
    }

    // TODO: find out if critical sections are necessary for the following two methods

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {       
        if(m_deviceSignalReceived) {
            m_deviceSignalReceived = false;
            emit executionFinished(m_outputData);
        }
        else {
            log("Waiting to receive response/trace data from scope...", TLogLevel::TInfo);
            m_waitingForExecutionFinish = true;
        }
    }

    void finishExecutionOrRememberSignalReceived() {
        if(m_waitingForExecutionFinish) {
            m_waitingForExecutionFinish = false;
            emit executionFinished(m_outputData);
        }
        else {
            log(
                "Response/trace data from scope received before block execution; "
                "is your sampling period long enough?",
                TLogLevel::TWarning
            );
            m_deviceSignalReceived = true;
        }
    }

    void runFailed() override {
        TScenarioScopeItem::runFailed();
        finishExecutionOrRememberSignalReceived();
    }

    void stopFailed() override {
        TScenarioScopeItem::stopFailed();
        finishExecutionOrRememberSignalReceived();
    }

    void downloadFailed() override {
        TScenarioScopeItem::downloadFailed();
        finishExecutionOrRememberSignalReceived();
    }

    void tracesEmpty() override {
        TScenarioScopeItem::tracesEmpty();
        finishExecutionOrRememberSignalReceived();
    }

    void stopped() override {
        TScenarioScopeItem::stopped();
        finishExecutionOrRememberSignalReceived();
    }

protected:
    bool m_deviceSignalReceived;
    bool m_waitingForExecutionFinish;
};

#endif // TSCENARIOSCOPESTOPITEM_H
