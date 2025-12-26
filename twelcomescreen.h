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

#ifndef TWELCOMESCREEN_H
#define TWELCOMESCREEN_H

#include <QWidget>
#include <QFrame>
#include <QLabel>

#include <QToolButton>

class TCenteredToolButton : public QToolButton
{
    Q_OBJECT
public:
    explicit TCenteredToolButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
};


class TWelcomeScreen : public QWidget
{
    Q_OBJECT
public:
    explicit TWelcomeScreen(QWidget *parent = nullptr);

    void setActions(QAction *newProject,
                    QAction *openProject,
                    QAction *saveProject,
                    QAction *closeProject,
                    QAction *openDevice,
                    QAction *protocolManager,
                    QAction *scenarioManager);

    QSize sizeHint() const override;

private:
    QFrame      *m_card;

    QLabel      *m_logoLabel;
    QLabel      *m_titleLabel;
    QLabel      *m_sloganLabel;

    QToolButton *m_btnNewProject;
    QToolButton *m_btnOpenProject;
    QToolButton *m_btnSaveProject;
    QToolButton *m_btnCloseProject;

    TCenteredToolButton *m_btnAddDevice;
    TCenteredToolButton *m_btnProtocolManager;
    TCenteredToolButton *m_btnScenarioManager;
};

#endif // TWELCOMESCREEN_H
