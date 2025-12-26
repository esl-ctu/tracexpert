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

#ifndef TPROJECTVIEW_H
#define TPROJECTVIEW_H

#include <QTreeView>
#include <QToolBar>

#include "../protocol/tprotocolmodel.h"
#include "../scenario/tscenariomodel.h"
#include "../pluginunit/component/tcomponentmodel.h"
#include "../pluginunit/io/tiodevicemodel.h"
#include "../pluginunit/scope/tscopemodel.h"

class TMainWindow;

class TProjectView : public QTreeView
{
    Q_OBJECT

public:
    TProjectView(QToolBar * toolbar, QWidget * parent = nullptr);

    virtual void setModel(QAbstractItemModel *model) override;

signals:
    void showIODeviceRequested(TIODeviceModel * IODevice);
    void showScopeRequested(TScopeModel * scope);
    void showAnalDeviceRequested(TAnalDeviceModel * analDevice);

protected slots:
    void selectionChanged(const QItemSelection & selected, const QItemSelection & deselected) override;

private slots:
    void createActions();

    void refreshActions();
    void refreshActionsForItem(TProjectItem * item);
    void showContextMenu(const QPoint & point);
    void runDefaultAction(const QModelIndex & index);

    void initComponent();
    void deinitComponent();
    void showComponentSettings();
    void openDevice();
    void addIODevice();
    void addScope();
    void addAnalDevice();

    void initIODevice();
    void deinitIODevice();
    void showIODevice();
    void removeIODevice();

    void initScope();
    void deinitScope();
    void showScope();
    void removeScope();

    void initAnalDevice();
    void deinitAnalDevice();
    void showAnalDevice();
    void removeAnalDevice();

    void showInfo();

    void showProtocolManager();
    void showScenarioManager();

    void openProtocolEditor();
    void openScenarioEditor();

private:
    QToolBar * m_toolbar;

    TComponentModel * m_component;
    TIODeviceModel * m_IODevice;
    TScopeModel * m_scope;
    TAnalDeviceModel * m_analDevice;
    TPluginUnitModel * m_unit;
    TProtocolModel * m_protocol;
    TScenarioModel * m_scenario;

    QList<QAction *> m_currentActions;
    QAction * m_currentDefaultAction;

    QAction * m_initComponentAction;
    QAction * m_deinitComponentAction;
    QAction * m_showComponentSettingsAction;
    QAction * m_openDeviceAction;
    QAction * m_addIODeviceAction;
    QAction * m_addScopeAction;
    QAction * m_addAnalDeviceAction;

    QAction * m_initIODeviceAction;
    QAction * m_deinitIODeviceAction;
    QAction * m_showIODeviceAction;
    QAction * m_removeIODeviceAction;

    QAction * m_initScopeAction;
    QAction * m_deinitScopeAction;
    QAction * m_showScopeAction;
    QAction * m_removeScopeAction;

    QAction * m_initAnalDeviceAction;
    QAction * m_deinitAnalDeviceAction;
    QAction * m_showAnalDeviceAction;
    QAction * m_removeAnalDeviceAction;

    QAction * m_openProtocolManagerAction;
    QAction * m_editProtocolAction;
    
    QAction * m_openScenarioManagerAction;
    QAction * m_editScenarioAction;
    
    QAction * m_showInfoAction;

    QAction * chooseDefaultAction(TComponentModel * component);
    QAction * chooseDefaultAction(TIODeviceModel * component);
    QAction * chooseDefaultAction(TScopeModel * component);
    QAction * chooseDefaultAction(TAnalDeviceModel * component);
};

#endif // TPROJECTVIEW_H
