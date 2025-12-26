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

#include "tactionwidget.h"

#include <QLayout>
#include <QLabel>
#include <QComboBox>

TActionWidget::TActionWidget(TAnalActionModel * actionModel, QWidget * parent)
    : QWidget(parent), m_actionModel(actionModel)
{
    m_runButton = new QPushButton(tr("Run"));
    m_runButton->setEnabled(m_actionModel->isEnabled());
    connect(m_runButton, &QPushButton::clicked, this, &TActionWidget::runAction);

    m_abortButton = new QPushButton(tr("Abort"));
    m_abortButton->setEnabled(false);
    connect(m_abortButton, &QPushButton::clicked, this, &TActionWidget::abortAction);

    QHBoxLayout * layout = new QHBoxLayout;
    layout->addWidget(m_runButton);
    layout->addWidget(m_abortButton);

    setLayout(layout);
}

void TActionWidget::actionStarted(TAnalActionModel * actionModel)
{
    m_runButton->setEnabled(false);
    if (actionModel == m_actionModel)
        m_abortButton->setEnabled(true);
}

void TActionWidget::actionFinished()
{
    m_runButton->setEnabled(m_actionModel->isEnabled());
    m_abortButton->setEnabled(false);
}

void TActionWidget::runAction()
{
    emit m_actionModel->run();
}

void TActionWidget::abortAction()
{
    emit m_actionModel->abort();
}
