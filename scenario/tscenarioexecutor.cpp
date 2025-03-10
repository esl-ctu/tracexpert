#include "tscenarioexecutor.h"
#include <QFutureWatcher>
#include <QtConcurrent>

#include "scenario_items/tscenariobasicitems.h"

TScenarioExecutor::TScenarioExecutor(TProjectModel * projectModel) {
    m_projectModel = projectModel;
}

TScenarioExecutor::~TScenarioExecutor() { }

void TScenarioExecutor::setScenario(TScenario * scenario) {
    m_scenario = scenario;

    m_prepareSuccessful = true;
    for(TScenarioItem * item : m_scenario->getItems()) {
        connect(item, &TScenarioItem::asyncLog, this, &TScenarioExecutor::log, Qt::BlockingQueuedConnection);
        connect(item, &TScenarioItem::syncLog, this, &TScenarioExecutor::log);
        item->setProjectModel(m_projectModel);
        item->updateParams(false);
        item->resetState();

        if(!item->prepare()) {
            m_prepareSuccessful = false;
            emit log("Failed to prepare block \"" + item->getName() + "\"!", "red");
        }
    }

    if(!m_prepareSuccessful) {
        for(TScenarioItem * item : m_scenario->getItems()) {
            disconnect(item, &TScenarioItem::asyncLog, this, &TScenarioExecutor::log);
            disconnect(item, &TScenarioItem::syncLog, this, &TScenarioExecutor::log);
        }
    }

    // create the connection map
    m_connectionMap.clear();
    for(TScenarioConnection * connection : m_scenario->getConnections()) {
        m_connectionMap.insert(connection->getSourcePort(), connection->getTargetPort());
    }
}

void TScenarioExecutor::haltExecution() {
    m_isRunning = false;

    for(TScenarioItem * item : m_scenario->getItems()) {        
        disconnect(item, nullptr, this, nullptr);
        if(!item->cleanup()) {
            emit log("Failed to cleanup block \"" + item->getName() + "\"!", "orange");
        }
    }
}

void TScenarioExecutor::stop() {
    m_isSupposedToRun = false;
}

void TScenarioExecutor::start() {
    if(!m_prepareSuccessful) {
        qWarning("Scenario cannot be executed, prepare step failed.");
        emit log("Scenario cannot be executed, prepare step failed.", "red");
        haltExecution();
        emit scenarioExecutionFinished();
        return;
    }

    if(m_isRunning) {
        qWarning("Scenario execution cannot be started while it is already running.");
        emit log("Scenario execution cannot be started while it is already running.", "red");
        haltExecution();
        emit scenarioExecutionFinished();
        return;
    }

    m_isRunning = true;
    m_isSupposedToRun = true;

    if(!m_scenario->validate()) {
        if(m_scenario->getState() != TScenario::TState::TOk) {
            emit log(m_scenario->getStateMessage(), "red");
        }

        qWarning("Failed to execute the scenario - validation failed. Fix scenario errors first.");
        emit log("Failed to execute the scenario - validation failed. Fix scenario errors first.", "red");
        haltExecution();
        emit scenarioExecutionFinished();
        return;
    }

    m_scenarioItemDataInputValues.clear();

    QFutureWatcher<void> * watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, &TScenarioExecutor::scenarioExecutionFinished);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
    watcher->setFuture((m_runFuture = QtConcurrent::run([this] {
        executeNonFlowItems();
        executeFlowItems();
    })));
}

void TScenarioExecutor::executeNonFlowItems() {
    // find items with no flow input ports (to be executed first), except flow end item...
    for(TScenarioItem * item : m_scenario->getItems()) {
        if(!m_isSupposedToRun) {
            qWarning("Scenario stopped during execution!");
            return;
        }

        if(!item->hasFlowInputPort() && item->itemClass() != TScenarioFlowStartItem::TItemClass) {
            m_currentItem = item;
            // execute item based on mode
            if(m_currentItem->supportsImmediateExecution()) {
                QHash<TScenarioItemPort *, QByteArray> outputData;
                outputData = m_currentItem->executeImmediate(m_scenarioItemDataInputValues.value(m_currentItem));
                saveOutputData(outputData);
            }
            else {
                m_blockExecutionFinished = false;
                connect(m_currentItem, &TScenarioItem::executionFinished, this, &TScenarioExecutor::blockExecutionFinished);
                connect(m_currentItem, &TScenarioItem::executionFinishedWithOutput, this, &TScenarioExecutor::blockExecutionFinishedWithOutput);
                m_currentItem->execute(m_scenarioItemDataInputValues.value(m_currentItem));

                while (!m_blockExecutionFinished) {
                    if(!m_isSupposedToRun) {
                        qWarning("Scenario was stopped during execution!");
                        emit log("Scenario was stopped during execution!", "red");
                        return;
                    }
                    QThread::yieldCurrentThread();
                }
            }
        }        
    }

    qDebug("Non-flow items execution finished!");
}

void TScenarioExecutor::executeFlowItems() {
    // find the flow start - first item
    m_currentItem = nullptr;
    for(TScenarioItem * item : m_scenario->getItems()) {
        if(item->getType() == TScenarioItem::TItemAppearance::TFlowStart) {
            m_currentItem = item;
            break;
        }
    }

    if(!m_currentItem) {
        qWarning("Failed to execute the scenario - no start block.");
        emit log("Failed to execute the scenario - no start block.", "red");
        haltExecution();
        return;
    }

    while(true) {
        if(!m_isSupposedToRun) {
            qWarning("Scenario was stopped during execution!");
            emit log("Scenario was stopped during execution!", "red");
            haltExecution();
            return;
        }

        if(!executeNextFlowItem()) {
            break;
        }
    }

    qDebug("Flow items execution finished!");
}

bool TScenarioExecutor::executeNextFlowItem() {
    TScenarioItem * previousItem = m_currentItem;

    TScenarioItemPort * currentItemFlowOutputPort = m_currentItem->getPreferredOutputFlowPort();
    if(!currentItemFlowOutputPort) {
        currentItemFlowOutputPort = getDefaultOutputFlowPort(m_currentItem);
    }

    TScenarioItemPort * nextItemFlowInputPort = m_connectionMap.value(currentItemFlowOutputPort);
    if(!nextItemFlowInputPort) {
        qWarning("Failed to execute the scenario - nowhere to go after executed block.");
        emit log("Failed to execute the scenario - nowhere to go after executed block.", "red");
        previousItem->setState(TScenarioItem::TState::TError, "Execution stopped here: unconnected output flow port!");
        haltExecution();
        return false;
    }

    m_currentItem = nextItemFlowInputPort->getParentItem();
    if(!m_currentItem) {
        qWarning("Failed to execute the scenario - nowhere to go after executed block.");
        emit log("Failed to execute the scenario - nowhere to go after executed block.", "red");
        previousItem->setState(TScenarioItem::TState::TError, "Execution stopped here: broken connection on an output flow port!");
        haltExecution();
        return false;
    }

    if(m_currentItem->getType() == TScenarioItem::TItemAppearance::TFlowEnd) {
        m_currentItem->setState(TScenarioItem::TState::TInfo, "Execution finished here successfully.");
        emit log("Scenario executed successfully.", "green");
        haltExecution();
        return false;
    }

    if(m_currentItem->supportsImmediateExecution()) {
        QHash<TScenarioItemPort *, QByteArray> outputData;
        outputData = m_currentItem->executeImmediate(m_scenarioItemDataInputValues.value(m_currentItem));
        saveOutputData(outputData);
    }
    else {
        m_blockExecutionFinished = false;
        connect(m_currentItem, &TScenarioItem::executionFinished, this, &TScenarioExecutor::blockExecutionFinished);
        connect(m_currentItem, &TScenarioItem::executionFinishedWithOutput, this, &TScenarioExecutor::blockExecutionFinishedWithOutput);
        m_currentItem->execute(m_scenarioItemDataInputValues.value(m_currentItem));

        while (!m_blockExecutionFinished) {
            if(!m_isSupposedToRun) {
                qWarning("Scenario was stopped during execution!");
                emit log("Scenario was stopped during execution!", "red");
                haltExecution();
                return false;
            }
            QCoreApplication::processEvents();
            QThread::yieldCurrentThread();
        }
    }

    return true;
}

void TScenarioExecutor::blockExecutionFinished() {
    disconnect(m_currentItem, &TScenarioItem::executionFinished, this, &TScenarioExecutor::blockExecutionFinished);
    disconnect(m_currentItem, &TScenarioItem::executionFinishedWithOutput, this, &TScenarioExecutor::blockExecutionFinishedWithOutput);
    m_blockExecutionFinished = true;
}

void TScenarioExecutor::blockExecutionFinishedWithOutput(QHash<TScenarioItemPort *, QByteArray> outputData) {
    disconnect(m_currentItem, &TScenarioItem::executionFinished, this, &TScenarioExecutor::blockExecutionFinished);
    disconnect(m_currentItem, &TScenarioItem::executionFinishedWithOutput, this, &TScenarioExecutor::blockExecutionFinishedWithOutput);
    saveOutputData(outputData);
    m_blockExecutionFinished = true;
}

void TScenarioExecutor::saveOutputData(QHash<TScenarioItemPort *, QByteArray> outputData) {
    // figure out where to send the data output values
    for (auto [sourceScenarioItemPort, value] : outputData.asKeyValueRange()) {
        if(!m_connectionMap.contains(sourceScenarioItemPort)) {
            qWarning("Failed to pass output data to next block - no connection.");
            emit log("Failed to pass output data to next block - no connection.", "orange");
            m_currentItem->setState(
                TScenarioItem::TState::TWarning,
                "Failed to pass data: unconnected output data port (" \
                + (sourceScenarioItemPort->getDisplayName().isEmpty() ? sourceScenarioItemPort->getName() : sourceScenarioItemPort->getDisplayName()) \
                + ")!"
            );
            continue;
        }

        TScenarioItemPort * destinationItemPort = m_connectionMap.value(sourceScenarioItemPort);
        TScenarioItem * destinationItem = destinationItemPort->getParentItem();

        QHash<TScenarioItemPort *, QByteArray>  data;
        if(m_scenarioItemDataInputValues.contains(destinationItem)) {
            data = m_scenarioItemDataInputValues.value(destinationItem);
        }

        data.insert(destinationItemPort, value);
        m_scenarioItemDataInputValues.insert(destinationItem, data);
    }
}

TScenarioItemPort * TScenarioExecutor::getDefaultOutputFlowPort(TScenarioItem * item) {
    for(TScenarioItemPort * p : item->getItemPorts()) {
        if (p->getType() == TScenarioItemPort::TItemPortType::TFlowPort &&
            p->getDirection() == TScenarioItemPort::TItemPortDirection::TOutputPort)
        {
            return p;
        }
    }

    return nullptr;
}
