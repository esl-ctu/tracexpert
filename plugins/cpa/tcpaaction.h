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
// Petr Socha (initial author)

#ifndef TCPAACTION_H
#define TCPAACTION_H

#include "tcpadevice.h"

class TCPAAction : public TAnalAction
{
public:
    explicit TCPAAction(QString name, QString info, std::function<void(void)> run);

    /// AnalAction name
    virtual QString getName() const override;
    /// AnalAction info
    virtual QString getInfo() const override;

    /// Get info on analytic action's availability
    virtual bool isEnabled() const override;
    /// Run the analytic action
    virtual void run() override;
    /// Abort the current run of analytic action
    virtual void abort() override;

private:
    QString m_name;
    QString m_info;
    std::function<void(void)> m_run;
};

#endif // TCPAACTION_H

