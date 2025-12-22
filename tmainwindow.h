#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <QMainWindow>
#include <QDir>

#include "graphs/tgraph.h"
#include "tdockmanager.h"
#include "project/tprojectmodel.h"
#include "project/tprojectview.h"
#include "logger/tloghandler.h"

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
    void createAnalDeviceDockWidget(TAnalDeviceModel * IODevice);

    void createProtocolManagerDockWidget();
    void createProtocolEditor(TProtocolModel * protocol);

    void createScenarioManagerDockWidget();
    void createScenarioEditorDockWidget(TScenarioModel * scenario);

    void createGraphDockWidget(TGraph * graph);

private slots:
    void showDeviceWizard();
    void showHdfWizard();

    void newProject();
    void openProject();
    void saveProject(bool saveAs = false);
    void saveProjectAs();
    bool closeProject();

    void showHelp();
    void showLicense();
    void showContributors();
    void showAbout();

private:
    void createMenus();
    void createActions();

    void createWelcome();
    void createLog(TLogWidget * logWidget);
    void createStatusBar(TLogLineWidget * logLineWidget);

    void readSettings();
    void writeSettings();

    void closeEvent(QCloseEvent * event) override;

    QAction * m_newProjectAction;
    QAction * m_openProjectAction;
    QAction * m_saveProjectAction;
    QAction * m_saveProjectAsAction;
    QAction * m_closeProjectAction;
    QAction * m_exitAction;

    QAction * m_openDeviceAction;
    QAction * m_openProtocolsAction;
    QAction * m_openScenariosAction;
    QAction * m_openHdfAction;

    QAction * m_helpAction;
    QAction * m_licenseAction;
    QAction * m_contributorsAction;
    QAction * m_aboutAction;

    QMenu * m_viewMenu;
    
    TProjectModel * m_projectModel;

    TProjectView * m_projectView;

    TDockManager * m_dockManager;

    TDockWidget * m_logDockWidget;
    TDockWidget * m_welcomeDockWidget;
    TDockWidget * m_projectDockWidget;
    TDockWidget * m_protocolManagerDockWidget;
    TDockWidget * m_scenarioManagerDockWidget;
    QList<TDockWidget *> m_scenarioEditorDockWidgets;
    QList<TDockWidget *> m_graphDockWidgets;

    QString m_projectFileName;
    QDir m_projectDirectory;
};
#endif // TMAINWINDOW_H
