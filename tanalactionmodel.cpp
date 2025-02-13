#include "tanalactionmodel.h"

TAnalActionModel::TAnalActionModel(TAnalAction * action, QObject * parent)
    : QObject(parent), m_action(action)
{
    m_runner = new TAnalActionRunner(m_action);
    m_aborter = new TAnalActionAborter(m_action);

    m_runnerThread = new QThread();
    m_aborterThread = new QThread();

    m_runner->moveToThread(m_runnerThread);
    m_aborter->moveToThread(m_aborterThread);

    connect(this, &TAnalActionModel::run, m_runner, &TAnalActionRunner::run, Qt::ConnectionType::QueuedConnection);
    connect(this, &TAnalActionModel::abort, m_aborter, &TAnalActionAborter::abort, Qt::ConnectionType::QueuedConnection);
    connect(m_runner, &TAnalActionRunner::started, this, &TAnalActionModel::started, Qt::ConnectionType::QueuedConnection);
    connect(m_runner, &TAnalActionRunner::finished, this, &TAnalActionModel::finished, Qt::ConnectionType::QueuedConnection);
    connect(m_runnerThread, &QThread::finished, m_runner, &QObject::deleteLater);
    connect(m_runnerThread, &QThread::finished, m_runnerThread, &QObject::deleteLater);
    connect(m_aborterThread, &QThread::finished, m_aborter, &QObject::deleteLater);
    connect(m_aborterThread, &QThread::finished, m_aborterThread, &QObject::deleteLater);

    m_runnerThread->start();
    m_aborterThread->start();
}

TAnalActionModel::~TAnalActionModel()
{
    m_runnerThread->quit();
    m_aborterThread->quit();
}

QString TAnalActionModel::name()
{
    return m_action->getName();
}

QString TAnalActionModel::info()
{
    return m_action->getInfo();
}

bool TAnalActionModel::isEnabled() const
{
    return m_action->isEnabled();
}
