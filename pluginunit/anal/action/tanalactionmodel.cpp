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
// Vojtěch Miškovský (initial author)

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
