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
// Vojtěch Miškovský

#ifndef TSCENARIOEXECUTOR_H
#define TSCENARIOEXECUTOR_H

#include <QObject>
#include <QFuture>

#include "../project/tprojectmodel.h"
#include "tscenario.h"

/*!
 * \brief The TScenarioExecutor class is responsible for executing a scenario.
 *
 * The class is responsible for executing a scenario.
 * It prepares the scenario, executes it and cleans up afterwards.
 *
 */
class TScenarioExecutor: public QObject {
    Q_OBJECT
public:
    TScenarioExecutor(TProjectModel * projectModel);
    ~TScenarioExecutor();

    void start(TScenario * scenario);
    void stop();
    void terminate();

signals:
    void scenarioExecutionFinished();
    void scenarioTerminationRequested();

private:    
    void setScenario(TScenario * scenario);
    bool prepareScenarioItems();

    void cleanupScenarioItems();
    void cleanupScenarioExecutionData();

    QFuture<void> m_runFuture;
    void runScenario();

    void executeNonFlowItems();
    void executeFlowItems();

    TScenarioItem * findNextFlowItem(TScenarioItem * currentItem);

    void executeItem(TScenarioItem * item);
    void executeItemDirectly(TScenarioItem * item);
    void executeItemIndirectly(TScenarioItem * item);

    void saveOutputData(QHash<TScenarioItemPort *, QByteArray> outputData);

    TScenarioItemPort * getDefaultOutputFlowPort(TScenarioItem * item);

    bool m_isRunning = false;
    std::atomic<bool> m_stopRequested = false;
    std::atomic<bool> m_terminationRequested = false;


    TScenario * m_scenario = nullptr;
    TProjectModel * m_projectModel = nullptr;

    QHash<TScenarioItem *, QHash<TScenarioItemPort *, QByteArray>> m_scenarioItemDataInputValues;
    QHash<TScenarioItemPort *, TScenarioItemPort *> m_flowConnectionMap;
    QHash<TScenarioItemPort *, QList<TScenarioItemPort *>> m_dataConnectionMap;
};

#endif // TSCENARIOEXECUTOR_H
