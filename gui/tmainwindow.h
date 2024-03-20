#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <QMainWindow>

#include "qdir.h"
#include "tdockmanager.h"
#include "tprojectmodel.h"
#include "tprojectview.h"

class TMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TMainWindow(QWidget * parent = nullptr);
    ~TMainWindow();

public slots:
    void createProjectDockWidget(TProjectModel * model);

    void createIODeviceDockWidget(TIODeviceModel * IODevice);
    void createScopeDockWidget(TScopeModel * scope);

    void createProtocolManagerWidget();
    void openProtocolEditor(const QString & protocolName);

    // void createScenarioEditorWidget();

private slots:
    void showDeviceWizard();

    void newProject();
    void openProject();
    void saveProject(bool saveAs = false);
    void saveProjectAs();
    void closeProject();

private:
    void createMenus();
    void createActions();

    void readSettings();
    void writeSettings();

    void closeEvent(QCloseEvent * event) override;

    QAction * m_newProjectAction;
    QAction * m_openProjectAction;
    QAction * m_saveProjectAction;
    QAction * m_saveProjectAsAction;
    QAction * m_closeProjectAction;

    QAction * m_openDeviceAction;

    QMenu * m_viewMenu;
    
    TProjectModel * m_projectModel;

    TProjectView * m_projectView;

    TDockManager * m_dockManager;

    TDockWidget * m_projectDockWidget;
    TDockWidget * m_protocolWidget;

    QString m_projectFileName;
    QDir m_projectDirectory;
};
#endif // TMAINWINDOW_H
