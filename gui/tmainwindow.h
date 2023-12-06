#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <QMainWindow>

#include "tdockmanager.h"
#include "tprojectmodel.h"

class TMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TMainWindow(QWidget * parent = nullptr);
    ~TMainWindow();

public slots:
    void createIODeviceDockWidget(TIODeviceModel * IODevice);
    void createScopeDockWidget(TScopeModel * scope);

private slots:
    void showDeviceWizard();

private:
    void createMenus();
    void createActions();

    QAction * m_newProjectAction;
    QAction * m_openProjectAction;
    QAction * m_saveProjectAction;
    QAction * m_saveProjectAsAction;

    QAction * m_openDeviceAction;
    
    TProjectModel * m_projectModel;

    TDockManager * m_dockManager;
};
#endif // TMAINWINDOW_H
