#ifndef TPROJECTVIEW_H
#define TPROJECTVIEW_H

#include <QTreeView>

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
    TProjectView(QWidget * parent = nullptr);

signals:
    void showIODeviceRequested(TIODeviceModel * IODevice);
    void showScopeRequested(TScopeModel * scope);
    void showAnalDeviceRequested(TAnalDeviceModel * analDevice);

private slots:
    void createActions();

    void showContextMenu(const QPoint &point);
    void runDefaultAction(const QModelIndex &index);

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

    void editProtocol();
    void editScenario();

private:
    TMainWindow * m_mainWindow;

    TComponentModel * m_component;
    TIODeviceModel * m_IODevice;
    TScopeModel * m_scope;
    TAnalDeviceModel * m_analDevice;
    TPluginUnitModel * m_unit;
    TProtocolModel * m_protocol;
    TScenarioModel * m_scenario;

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
