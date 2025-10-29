#include "tscenarioscopestopitem.h"
#include "tscenarioscopestartitem.h"

bool TScenarioScopeStopItem::shouldUpdateParams(TConfigParam newParams) {
   return false;
}

void TScenarioScopeStopItem::updateParams(bool paramValuesChanged) { }

bool TScenarioScopeStopItem::prepare() {
    if(!this->getItemPortByName("startConnection")->hasConnectedPort()) {
        setState(TState::TError, tr("Failed to get \"start measurement\" item, is it connected?"));
        return false;
    }

    QList<TScenarioItemPort*> connectedItemPorts = this->getItemPortByName("startConnection")->getConnectedPorts();
    TScenarioItem * connectedItem = (connectedItemPorts.first())->getParentItem();

    if(connectedItem->itemClass() != TScenarioScopeStartItem::TItemClass) {
        setState(TState::TError, tr("Failed to get \"start measurement\" item, a wrong item is connected!"));
        return false;
    }

    m_startItem = (TScenarioScopeStartItem *)connectedItem;
    m_scopeModel = m_startItem->getScopeModel();

    // ignore error if we cannot access the scope model,
    // the start measurement block takes responsibility for that error
    // if(!m_scopeModel)

    return true;
}

void TScenarioScopeStopItem::executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) {
    if(!m_outputData.empty()) {
        emit executionFinished();
    }
}
