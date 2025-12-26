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

#ifndef TACTIONWIDGET_H
#define TACTIONWIDGET_H

#include <QPushButton>

#include "../../anal/action/tanalactionmodel.h"

class TActionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TActionWidget(TAnalActionModel * actionModel, QWidget * parent = nullptr);

public slots:
    void actionStarted(TAnalActionModel * actionModel);
    void actionFinished();

private slots:
    void runAction();
    void abortAction();

private:
    TAnalActionModel * m_actionModel;

    QPushButton * m_runButton;
    QPushButton * m_abortButton;
};

#endif // TACTIONWIDGET_H
