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
    void log(const QString & message, const QString & color = "black");

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
