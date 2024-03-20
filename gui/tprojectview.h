#ifndef TPROJECTVIEW_H
#define TPROJECTVIEW_H

#include <QTreeView>

#include "protocol/tprotocolmodel.h"
#include "tcomponentmodel.h"
#include "tiodevicemodel.h"
#include "tscopemodel.h"

class TMainWindow;

class TProjectView : public QTreeView
{
    Q_OBJECT

public:
    TProjectView(QWidget * parent = nullptr);

signals:
    void showIODeviceRequested(TIODeviceModel * IODevice);
    void showScopeRequested(TScopeModel * scope);

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

    void initIODevice();
    void deinitIODevice();
    void showIODevice();

    void initScope();
    void deinitScope();
    void showScope();

    void editProtocol();

private:
    TMainWindow * m_mainWindow;

    TComponentModel * m_component;
    TIODeviceModel * m_IODevice;
    TScopeModel * m_scope;
    TProtocolModel * m_protocol;

    QAction * m_initComponentAction;
    QAction * m_deinitComponentAction;
    QAction * m_showComponentSettingsAction;
    QAction * m_openDeviceAction;
    QAction * m_addIODeviceAction;
    QAction * m_addScopeAction;

    QAction * m_initIODeviceAction;
    QAction * m_deinitIODeviceAction;
    QAction * m_showIODeviceAction;

    QAction * m_initScopeAction;
    QAction * m_deinitScopeAction;
    QAction * m_showScopeAction;

    QAction * m_openProtocolManagerAction;
    QAction * m_editProtocolAction;

    QAction * chooseDefaultAction(TComponentModel * component);
    QAction * chooseDefaultAction(TIODeviceModel * component);
    QAction * chooseDefaultAction(TScopeModel * component);
};

#endif // TPROJECTVIEW_H
