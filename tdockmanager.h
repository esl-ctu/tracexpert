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
// Adam Švehla

#ifndef TDOCKMANAGER_H
#define TDOCKMANAGER_H

#define USE_ADS

#ifdef USE_ADS

#include "DockManager.h"

typedef ads::CDockManager TDockManagerBase;
typedef ads::CDockWidget TDockWidgetBase;
#define TDockArea ads::DockWidgetArea

#else

#include <QDockWidget>
#include <QMainWindow>

typedef QMainWindow TDockManagerBase;
typedef QDockWidget TDockWidgetBase;
#define TDockArea Qt::DockWidgetArea

#endif

class TDockWidget : public TDockWidgetBase
{
    Q_OBJECT

public:
    explicit TDockWidget(const QString & title, QWidget * parent = nullptr);

    void setDeleteOnClose(bool value);

#ifdef USE_ADS
    void setWidget(QWidget* widget, eInsertMode InsertMode = AutoScrollArea);
#endif

#ifndef USE_ADS
    bool isClosed();
    void closeEvent(QCloseEvent *event) override;
#endif

public slots:
    void show();
    void close();


signals:
#ifndef USE_ADS
    void closed();
#endif

};

class TDockManager
#ifdef USE_ADS
    : public TDockManagerBase
#endif
{

public:
    explicit TDockManager(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

#ifndef USE_ADS
    void addDockWidget(TDockArea dockArea, TDockWidget * dockWidget);
    void removeDockWidget(TDockWidget * dockWidget);
#endif

    void addCenterDockWidget(TDockWidget * dockWidget);
    void addCenterDockWidgetTab(TDockWidget * dockWidget, TDockWidget * existingDockWidget);

#ifndef USE_ADS
protected:
    TDockManagerBase * m_mainWindow = nullptr;
#endif
};

#endif // TDOCKMANAGER_H
