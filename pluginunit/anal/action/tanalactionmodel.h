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

#ifndef TANALACTIONMODEL_H
#define TANALACTIONMODEL_H

#include <QThread>

#include "tanaldevice.h"
#include "tanalactionrunner.h"
#include "tanalactionaborter.h"

class TAnalActionModel : public QObject
{
    Q_OBJECT

public:
    explicit TAnalActionModel(TAnalAction * action, QObject * parent = nullptr);
    ~TAnalActionModel();

    QString name();
    QString info();

    bool isEnabled() const;

private:
    TAnalAction * m_action;
    TAnalActionRunner * m_runner;
    TAnalActionAborter * m_aborter;

    QThread * m_runnerThread;
    QThread * m_aborterThread;

signals:
    void run();
    void abort();
    void started();
    void finished();
};

#endif // TANALACTIONMODEL_H
