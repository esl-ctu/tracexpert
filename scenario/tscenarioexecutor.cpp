#include "tscenarioexecutor.h"
#include <QFutureWatcher>
#include <QtConcurrent>

#include "scenario_items/tscenariobasicitems.h"
#include "scenario_items/tscenariovariablereaditem.h"
#include "scenario_items/tscenariovariablewriteitem.h"
#include "tscenarioexecutionexceptions.h"


TScenarioExecutor::TScenarioExecutor(TProjectModel * projectModel) {
    m_projectModel = projectModel;
}

TScenarioExecutor::~TScenarioExecutor() { }

void TScenarioExecutor::setScenario(TScenario * scenario) {
    m_scenario = scenario;

    QHash<QString, QList<TScenarioItemPort *>> variableDataInPorts;
    QHash<QString, QList<TScenarioItemPort *>> variableDataOutPorts;

    for(TScenarioItem * item : m_scenario->getItems()) {
        // clear all ports' "connected ports"
        for(TScenarioItemPort * itemPort : item->getItemPorts()) {
            itemPort->clearConnectedPorts();
        }

        // gather all variable write items and their dataIn ports
        if(item->itemClass() == TScenarioItem::TItemClass::TScenarioVariableWriteItem) {
            QString variableName = ((TScenarioVariableWriteItem*)item)->variableName();

            QList<TScenarioItemPort *> ports;
            if(variableDataInPorts.contains(variableName)) {
                ports = variableDataInPorts.value(variableName);
            }

            ports.append(item->getItemPortByName("dataIn"));
            variableDataInPorts.insert(variableName, ports);
        }
        // gather all variable read items and their dataOut ports
        else if(item->itemClass() == TScenarioItem::TItemClass::TScenarioVariableReadItem) {
            QString variableName = ((TScenarioVariableReadItem*)item)->variableName();

            QList<TScenarioItemPort *> ports;
            if(variableDataOutPorts.contains(variableName)) {
                ports = variableDataOutPorts.value(variableName);
            }

            ports.append(item->getItemPortByName("dataOut"));
            variableDataOutPorts.insert(variableName, ports);
        }
    }

    // create preliminary data connection map
    QHash<TScenarioItemPort *, TScenarioItemPort *> dataConnectionMap;
    for(TScenarioConnection * connection : m_scenario->getConnections()) {
        if(connection->getTargetPort()->getType() == TScenarioItemPort::TItemPortType::TDataPort) {
            dataConnectionMap.insert(connection->getSourcePort(), connection->getTargetPort());
        }
    }

    // create map that makes variable r&w blocks act transparent (as connections)
    // all "write to variable" destination ports are rerouted to "read variable" destination ports
    // for each variable
    QHash<TScenarioItemPort *, QList<TScenarioItemPort *>> dataInToDataOutDestinationConnectionMap;
    for(auto [variableName, dataOutPorts] : variableDataOutPorts.asKeyValueRange()) {
        QList<TScenarioItemPort *> dataOutDestinationPorts;
        for(TScenarioItemPort * dataOutPort : dataOutPorts) {
            dataOutDestinationPorts.append(dataConnectionMap.value(dataOutPort));
        }

        for(TScenarioItemPort * dataInPort : variableDataInPorts.value(variableName)) {
            dataInToDataOutDestinationConnectionMap.insert(dataInPort, dataOutDestinationPorts);
        }
    }

    // create the flow and data connection map
    m_flowConnectionMap.clear();
    m_dataConnectionMap.clear();
    for(TScenarioConnection * connection : m_scenario->getConnections()) {

        // add connection information to the ports themselves
        connection->getSourcePort()->addConnectedPort(connection->getTargetPort());
        connection->getTargetPort()->addConnectedPort(connection->getSourcePort());

        if(connection->getSourcePort()->getType() == TScenarioItemPort::TItemPortType::TFlowPort) {
            m_flowConnectionMap.insert(connection->getSourcePort(), connection->getTargetPort());            
        }
        else {
            QList<TScenarioItemPort *> ports;
            if(m_dataConnectionMap.contains(connection->getSourcePort())) {
                ports = m_dataConnectionMap.value(connection->getSourcePort());
            }

            // add virtual variable connections to the data connection map
            if(dataInToDataOutDestinationConnectionMap.contains(connection->getTargetPort())) {
                ports.append(dataInToDataOutDestinationConnectionMap.value(connection->getTargetPort()));
            }
            else {
                ports.append(connection->getTargetPort());
            }

            m_dataConnectionMap.insert(connection->getSourcePort(), ports);
        }
    }
}

bool TScenarioExecutor::prepareScenarioItems() {
    bool prepareSuccessful = true;
    for(TScenarioItem * item : m_scenario->getItems()) {
        try {
            item->setProjectModel(m_projectModel);
            item->updateParams(false);
            item->resetState(true);

            if(!item->prepare()) {
                prepareSuccessful = false;
                qWarning("Failed to prepare block \"%s\"!", qPrintable(item->getName()));
            }
        }
        catch(...) {
            prepareSuccessful = false;
            qWarning("Failed to prepare block \"%s\", exception thrown!", qPrintable(item->getName()));
        }
    }

    if(!prepareSuccessful) {
        cleanupScenarioItems();
    }

    return prepareSuccessful;
}

void TScenarioExecutor::cleanupScenarioItems() {
    for(TScenarioItem * item : m_scenario->getItems()) {
        try {
            if(!item->cleanup()) {
                qWarning("Failed to cleanup after block \"%s\"!", qPrintable(item->getName()));
            }
        }
        catch(...) {
            qWarning("Failed to cleanup after block \"%s\", exception thrown!", qPrintable(item->getName()));
        }
    }
}

void TScenarioExecutor::cleanupScenarioExecutionData() {
    m_scenarioItemDataInputValues.clear();
}

void TScenarioExecutor::stop() {
    m_stopRequested = true;
}

void TScenarioExecutor::terminate() {
    m_terminationRequested = true;
}

void TScenarioExecutor::start(TScenario * scenario) {

    if(m_isRunning) {
        qWarning("Scenario execution cannot be started while it is already running.");
        return;
    }

    setScenario(scenario);

    if(!prepareScenarioItems()) {
        qWarning("Scenario cannot be executed, prepare step failed.");
        emit scenarioExecutionFinished();
        return;
    }   

    m_stopRequested = false;
    m_terminationRequested = false;

    if(!m_scenario->validate()) {
        if(m_scenario->getState() != TScenario::TState::TOk) {
            qWarning("Scenario validation result: %s", qPrintable(m_scenario->getStateMessage()));
        }

        qWarning("Scenario cannot be executed, validation failed. Fix scenario errors first.");

        cleanupScenarioItems();
        emit scenarioExecutionFinished();
        return;
    }

    // clean execution data possibly left by a previous run
    cleanupScenarioExecutionData();

    QFutureWatcher<void> * watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, watcher, &QFutureWatcher<void>::deleteLater);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this]() {
        m_isRunning = false;
        emit scenarioExecutionFinished();
    });

    m_runFuture = QtConcurrent::run(&TScenarioExecutor::runScenario, this);
    watcher->setFuture(m_runFuture);
}

void TScenarioExecutor::runScenario() {
    m_isRunning = true;
    qInfo("Scenario execution started.");

    try {
        executeNonFlowItems();
        executeFlowItems();

        qInfo("Scenario execution finished successfully.");
    }
    catch(ScenarioExecutionException &) {
        qWarning("Scenario cannot continue execution, halting!");
    }
    catch(ScenarioHaltRequestedException &) {
        qWarning("Scenario stop was requested by the user, halting!");
    }
    catch(...) {
        qWarning("An unhandled exception occured during scenario execution, halting!");
    }

    try {
        cleanupScenarioItems();
    }
    catch(...) {
        qWarning("Failed to clean up after scenario!");
    }

    cleanupScenarioExecutionData();
}

void TScenarioExecutor::executeNonFlowItems() {
    // find items with no flow input ports (to be executed first), except flow end item...
    for(TScenarioItem * item : m_scenario->getItems()) {
        if(m_stopRequested || m_terminationRequested)
            throw ScenarioHaltRequestedException();

        if(!item->hasFlowInputPort() && item->itemClass() != TScenarioItem::TItemClass::TScenarioFlowStartItem) {
            executeItem(item);
        }        
    }
}

void TScenarioExecutor::executeItem(TScenarioItem * item) {
    item->setState(TScenarioItem::TState::TBeingExecuted, "This block is being executed.");

    if(item->supportsDirectExecution()) {
        executeItemDirectly(item);
    }
    else {
        executeItemIndirectly(item);
    }

    if(item->getState() == TScenarioItem::TState::TBeingExecuted) {
        item->resetState();
    }
}

void TScenarioExecutor::executeItemDirectly(TScenarioItem * item) {
    QHash<TScenarioItemPort *, QByteArray> inputData, outputData;

    inputData = m_scenarioItemDataInputValues.value(item);
    outputData = item->executeDirect(inputData);

    saveOutputData(outputData);
}

void TScenarioExecutor::executeItemIndirectly(TScenarioItem * item) {
    bool executionFinished = false;

    QEventLoop loop;
    connect(item, &TScenarioItem::executionFinished, &loop, &QEventLoop::quit);
    connect(item, &TScenarioItem::executionFinished, this, [&executionFinished, this](QHash<TScenarioItemPort *, QByteArray> outputData) {
        saveOutputData(outputData);
        executionFinished = true;
    });

    try {
        item->executeIndirect(m_scenarioItemDataInputValues.value(item));
    }
    catch (...) {
        disconnect(item, &TScenarioItem::executionFinished, nullptr, nullptr);

        qWarning("An exception occurred while waiting for block execution to finish.");
        throw ScenarioExecutionException();
    }

    bool cancelCalled = false;

    while (!executionFinished) {
        QTimer::singleShot(1000, &loop, &QEventLoop::quit);
        loop.exec();

        if(m_stopRequested && !cancelCalled) {
            try {
                item->stopExecution();
                qWarning() << "An attempt at stopping scenario block execution was made; "
                            << "to terminate forcefully, press the stop button again.";
            }
            catch(...) {
                qWarning() << "An attempt at stopping scenario block execution failed; "
                           << "to terminate forcefully, press the stop button again.";
            }

            cancelCalled = true;
        }

        if(m_terminationRequested) {
            try {
                item->terminateExecution();
                qWarning() << "Scenario block execution was terminated forcefully.";
            }
            catch(...) {
                qWarning() << "An attempt at forceful scenario block execution termination failed; "
                           << "the program may be in an undefined state.";
            }

            break;
        }
    }

    disconnect(item, &TScenarioItem::executionFinished, nullptr, nullptr);
}

void TScenarioExecutor::executeFlowItems() {
    // find the flow start - first item
    TScenarioItem * firstItem = nullptr;
    for(TScenarioItem * item : m_scenario->getItems()) {
        if(item->getType() == TScenarioItem::TItemAppearance::TFlowStart) {
            firstItem = item;
            break;
        }
    }

    if(!firstItem) {
        qWarning("Failed to execute the scenario - no start block.");
        throw ScenarioExecutionException();
    }

    TScenarioItem * currentItem = findNextFlowItem(firstItem);

    while(currentItem) {
        if(m_stopRequested || m_terminationRequested)
            throw ScenarioHaltRequestedException();

        if(currentItem->getType() == TScenarioItem::TItemAppearance::TFlowEnd) {
            currentItem->setState(TScenarioItem::TState::TRuntimeInfo, "Execution finished here successfully.");
            break;
        }

        currentItem->resetState(true);
        executeItem(currentItem);
        currentItem = findNextFlowItem(currentItem);
    }
}

TScenarioItem * TScenarioExecutor::findNextFlowItem(TScenarioItem * item) {

    TScenarioItemPort * currentItemFlowOutputPort = item->getPreferredOutputFlowPort();
    if(!currentItemFlowOutputPort) {
        currentItemFlowOutputPort = getDefaultOutputFlowPort(item);
    }

    TScenarioItemPort * nextItemFlowInputPort = m_flowConnectionMap.value(currentItemFlowOutputPort);
    if(!nextItemFlowInputPort) {
        item->setState(
            TScenarioItem::TState::TRuntimeError,
            QString("Execution stopped here: unconnected output flow port (%1)!").arg(currentItemFlowOutputPort->getLabelText())
        );
        qWarning("Failed to execute the scenario - nowhere to go after executed block.");

        throw ScenarioExecutionException();
    }

    TScenarioItem * nextItem = nextItemFlowInputPort->getParentItem();
    if(!nextItem) {
        item->setState(TScenarioItem::TState::TRuntimeError, "Execution stopped here: broken connection on an output flow port!");
        qWarning("Failed to execute the scenario - nowhere to go after executed block.");

        throw ScenarioExecutionException();
    }

    return nextItem;
}

void TScenarioExecutor::saveOutputData(QHash<TScenarioItemPort *, QByteArray> outputData) {
    // figure out where to send the data output values
    for (auto [sourceItemPort, value] : outputData.asKeyValueRange()) {
        if(!m_dataConnectionMap.contains(sourceItemPort)) {
            QString portName = sourceItemPort->getLabelText().isEmpty() ? sourceItemPort->getName() : sourceItemPort->getLabelText();
            qInfo() << "Output data could not be passed to next block: "
                    << "unconnected output data port (" << portName << ") in block " << sourceItemPort->getParentItem()->getName() << ".";
            sourceItemPort->getParentItem()->setState(
                TScenarioItem::TState::TRuntimeWarning,
                "Output data could not be passed to next block: unconnected output data port (" + portName + ")!"
            );
            continue;
        }

        for(TScenarioItemPort * destinationItemPort : m_dataConnectionMap.value(sourceItemPort)) {
            TScenarioItem * destinationItem = destinationItemPort->getParentItem();

            QHash<TScenarioItemPort *, QByteArray>  data;
            if(m_scenarioItemDataInputValues.contains(destinationItem)) {
                data = m_scenarioItemDataInputValues.value(destinationItem);
            }

            data.insert(destinationItemPort, value);
            m_scenarioItemDataInputValues.insert(destinationItem, data);
        }
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
