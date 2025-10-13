#include "tscenarioscopestartitem.h"
#include "tscenarioscopestopitem.h"

bool TScenarioScopeStartItem::prepare() {
    m_scopeModel = getScopeModel();

    if(!m_scopeModel) {
        setState(TState::TError, tr("Failed to obtain selected Oscilloscope, is it available?"));
        return false;
    }

    if(!checkAndSetInitParamsAtPreparation()) {
        return false;
    }

    if(!this->getItemPortByName("stopConnection")->hasConnectedPort()) {
        setState(TState::TError, tr("Failed to get \"stop measurement\" item, is it connected?"));
        return false;
    }

    QList<TScenarioItemPort*> connectedItemPorts = this->getItemPortByName("stopConnection")->getConnectedPorts();
    TScenarioItem * connectedItem = (connectedItemPorts.first())->getParentItem();

    if(connectedItem->itemClass() != TScenarioScopeStopItem::TItemClass) {
        setState(TState::TError, tr("Failed to get \"stop measurement\" item, a wrong item is connected!"));
        return false;
    }

    m_stopItem = (TScenarioScopeStopItem *)connectedItem;

    return true;
}


QHash<TScenarioItemPort *, QByteArray> TScenarioScopeStartItem::executeImmediate(const QHash<TScenarioItemPort *, QByteArray> & inputData) {
    checkAndSetInitParamsBeforeExecution();

    connect(m_scopeModel, &TScopeModel::runFailed, this, &TScenarioScopeItem::runFailed);

    m_stopItem->clearOutputData();

    // the following events are connected to the connected block instance
    connect(m_scopeModel, &TScopeModel::stopFailed, m_stopItem, &TScenarioScopeItem::stopFailed);
    connect(m_scopeModel, &TScopeModel::downloadFailed, m_stopItem, &TScenarioScopeItem::downloadFailed);

    connect(m_scopeModel, &TScopeModel::tracesDownloaded, m_stopItem, &TScenarioScopeItem::tracesDownloaded);
    connect(m_scopeModel, &TScopeModel::tracesEmpty, m_stopItem, &TScenarioScopeItem::tracesEmpty);
    connect(m_scopeModel, &TScopeModel::stopped, m_stopItem, &TScenarioScopeItem::stopped);

    m_scopeModel->runSingle();
    log(QString(tr("[%1] Measurement started")).arg(m_scopeModel->name()));

    return {};
}
