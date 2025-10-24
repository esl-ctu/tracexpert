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

    void setScenario(TScenario * scenario);

    void start();
    void stop();

public slots:
    void blockExecutionFinished();
    void blockExecutionFinishedWithOutput(QHash<TScenarioItemPort *, QByteArray> outputData);

signals:
    void scenarioExecutionFinished();
    void log(const QString & message, const QString & color = "black");

private:
    QFuture<void> m_runFuture;

    void executeNonFlowItems();
    void executeFlowItems();

    bool executeNextFlowItem();

    bool executeCurrentItem();

    void haltExecution();

    void saveOutputData(QHash<TScenarioItemPort *, QByteArray> outputData);
    bool m_blockExecutionFinished;

    TScenarioItemPort * getDefaultOutputFlowPort(TScenarioItem * item);

    bool m_prepareSuccessful = false;
    bool m_isRunning = false;
    std::atomic<bool> m_isSupposedToRun = false;

    TScenario * m_scenario = nullptr;
    TProjectModel * m_projectModel = nullptr;

    QHash<TScenarioItem *, QHash<TScenarioItemPort *, QByteArray>> m_scenarioItemDataInputValues;
    QHash<TScenarioItemPort *, TScenarioItemPort *> m_flowConnectionMap;
    QHash<TScenarioItemPort *, QList<TScenarioItemPort *>> m_dataConnectionMap;

    TScenarioItem * m_currentItem;
};

#endif // TSCENARIOEXECUTOR_H
