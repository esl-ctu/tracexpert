#include <QMenu>
#include <QMenuBar>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QOpenGLWidget>
#include <QHelpEngine>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QHelpLink>
#include <qstatusbar.h>

#include "tmainwindow.h"
#include "graphs/tgraph.h"
#include "graphs/tgraphwidget.h"
#include "qfiledialog.h"
#include "tdevicewizard.h"
#include "pluginunit/common/widgets/tcommunicationdevicewidget.h"
#include "scenario/tscenariowidget.h"
#include "scenario/tscenarioeditorwidget.h"
#include "pluginunit/scope/tscopewidget.h"
#include "project/tprojectview.h"
#include "project/tprojectmodel.h"
#include "protocol/tprotocolwidget.h"
#include "tdialog.h"
#include "thelpbrowser.h"

TMainWindow::TMainWindow(TLogHandler * logHandler, QWidget * parent)
    : QMainWindow(parent)
{
    // Workaround to prevent window reopening when oscilloscope widget is loaded for the first time
    QOpenGLWidget w;
    setCentralWidget(&w);

    setCentralWidget(nullptr);

    m_dockManager = new TDockManager(this);
    m_projectModel = nullptr;
    m_projectView = nullptr;

    m_welcomeDockWidget = nullptr;
    m_protocolManagerDockWidget = nullptr;
    m_projectDockWidget = nullptr;
    m_scenarioManagerDockWidget = nullptr;

    createActions();
    createMenus();

    readSettings();

    createWelcome();
    createLog(logHandler->logWidget());
    createStatusBar(logHandler->logLineWidget());
}

TMainWindow::~TMainWindow()
{

}

void TMainWindow::createMenus()
{
    QMenu * fileMenu = new QMenu(tr("&File"), this);
    fileMenu->addAction(m_newProjectAction);
    fileMenu->addAction(m_openProjectAction);
    fileMenu->addAction(m_saveProjectAction);
    fileMenu->addAction(m_saveProjectAsAction);
    fileMenu->addAction(m_closeProjectAction);
    menuBar()->addMenu(fileMenu);

    m_viewMenu = new QMenu(tr("View"), this);
    menuBar()->addMenu(m_viewMenu);

    QMenu * devicesMenu = new QMenu(tr("Devices"), this);
    devicesMenu->addAction(m_openDeviceAction);
    menuBar()->addMenu(devicesMenu);

    QMenu * helpMenu = new QMenu(tr("Help"), this);

    QString helpPath = QApplication::applicationDirPath() + "/docs/docs.qhc";
    if (QFile::exists(helpPath))
        helpMenu->addAction(m_helpAction);

    helpMenu->addAction(m_aboutAction);
    menuBar()->addMenu(helpMenu);
}

void TMainWindow::createActions()
{
    m_newProjectAction = new QAction(tr("&New project"), this);
    m_newProjectAction->setShortcuts(QKeySequence::New);
    m_newProjectAction->setStatusTip(tr("Create a new project"));
    m_newProjectAction->setIcon(QIcon::fromTheme("document-new"));
    m_newProjectAction->setIconVisibleInMenu(true);
    connect(m_newProjectAction, SIGNAL(triggered()), this, SLOT(newProject()));

    m_openProjectAction = new QAction(tr("&Open project"), this);
    m_openProjectAction->setShortcuts(QKeySequence::Open);
    m_openProjectAction->setStatusTip(tr("Open an existing project"));
    m_openProjectAction->setIcon(QIcon::fromTheme("document-open"));
    connect(m_openProjectAction, SIGNAL(triggered()), this, SLOT(openProject()));

    m_saveProjectAction = new QAction(tr("&Save project"), this);
    m_saveProjectAction->setShortcuts(QKeySequence::Save);
    m_saveProjectAction->setIcon(QIcon::fromTheme("document-save"));
    m_saveProjectAction->setStatusTip(tr("Save the currect project"));
    m_saveProjectAction->setEnabled(false);
    connect(m_saveProjectAction, SIGNAL(triggered()), this, SLOT(saveProject()));

    m_saveProjectAsAction = new QAction(tr("&Save project as"), this);
    m_saveProjectAsAction->setShortcuts(QKeySequence::SaveAs);
    m_saveProjectAsAction->setIcon(QIcon::fromTheme("document-save-as"));
    m_saveProjectAsAction->setStatusTip(tr("Select path and save the current project"));
    m_saveProjectAsAction->setEnabled(false);
    connect(m_saveProjectAsAction, SIGNAL(triggered()), this, SLOT(saveProjectAs()));

    m_closeProjectAction = new QAction(tr("&Close project"), this);
    m_closeProjectAction->setShortcuts(QKeySequence::Close);
    m_closeProjectAction->setIcon(QIcon::fromTheme("document-close"));
    m_closeProjectAction->setStatusTip(tr("Close the current project"));
    m_closeProjectAction->setEnabled(false);
    connect(m_closeProjectAction, SIGNAL(triggered()), this, SLOT(closeProject()));

    m_openDeviceAction = new QAction(tr("Open device"), this);
    m_openDeviceAction->setStatusTip(tr("Open a device using device wizard"));
    m_openDeviceAction->setEnabled(false);
    connect(m_openDeviceAction, SIGNAL(triggered()), this, SLOT(showDeviceWizard()));

    m_helpAction = new QAction(tr("Help"), this);
    m_helpAction->setStatusTip(tr("Show help"));
    connect(m_helpAction, SIGNAL(triggered()), this, SLOT(showHelp()));

    m_aboutAction = new QAction(tr("About"), this);
    m_aboutAction->setStatusTip(tr("Show information about this program"));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void TMainWindow::createWelcome() {
    m_welcomeDockWidget = new TDockWidget(tr("Welcome"));

    QLabel * imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setPixmap(QPixmap(":/icons/tracexpert512.png"));

    m_welcomeDockWidget->setWidget(imageLabel);
    m_dockManager->addCenterDockWidget(m_welcomeDockWidget);
}

void TMainWindow::createLog(TLogWidget * logWidget) {
    m_logDockWidget = new TDockWidget(tr("Log"));
    logWidget->setReadOnly(true);
    logWidget->setMinimumHeight(100);
    m_logDockWidget->setWidget(logWidget);
    m_viewMenu->addAction(m_logDockWidget->toggleViewAction());
    m_dockManager->addDockWidget(TDockArea::BottomDockWidgetArea, m_logDockWidget);
    m_logDockWidget->close();
}

void TMainWindow::createStatusBar(TLogLineWidget * logLineWidget) {
    QStatusBar * statusBar = this->statusBar();

    connect(logLineWidget, &TLogLineWidget::clicked, this, [this]() {
        m_logDockWidget->isClosed() ? m_logDockWidget->show() : m_logDockWidget->close();

    });

    logLineWidget->setStatusTip(tr("Toggle the Log widget"));
    logLineWidget->setText("Log messages will appear here. <i>You can click on them to show or hide the Log widget.</i>");
    statusBar->addPermanentWidget(logLineWidget);
}

void TMainWindow::createProjectDockWidget(TProjectModel * model)
{
    m_projectDockWidget = new TDockWidget(tr("Project"), this);
    m_projectView = new TProjectView(this);
    m_projectView->setMinimumWidth(450);
    m_projectModel = model;
    m_projectDirectory = QDir::current();
    m_projectView->setModel(m_projectModel);
    m_projectView->expandToDepth(1);
    m_projectView->resizeColumnToContents(0);
    m_projectDockWidget->setWidget(m_projectView);
    m_dockManager->addDockWidget(TDockArea::LeftDockWidgetArea, m_projectDockWidget);
    m_viewMenu->insertAction(m_viewMenu->actions().isEmpty() ? nullptr : m_viewMenu->actions().constFirst(), m_projectDockWidget->toggleViewAction());

    m_saveProjectAction->setEnabled(true);
    m_saveProjectAsAction->setEnabled(true);
    m_closeProjectAction->setEnabled(true);
    m_openDeviceAction->setEnabled(true);
}

void TMainWindow::createIODeviceDockWidget(TIODeviceModel * IODevice)
{
    TCommunicationDeviceWidget * widget = new TCommunicationDeviceWidget(IODevice, m_projectModel->protocolContainer());
    QString title = widget->windowTitle();
    TDockWidget * dockWidget = new TDockWidget(title);
    dockWidget->setWidget(widget);
    m_viewMenu->addAction(dockWidget->toggleViewAction());
    connect(IODevice, &TIODeviceModel::deinitialized, dockWidget, [=](){ m_viewMenu->removeAction(dockWidget->toggleViewAction()); });
    connect(IODevice, &TIODeviceModel::deinitialized, dockWidget, [=](){ m_dockManager->removeDockWidget(dockWidget); });
    connect(IODevice, &TIODeviceModel::deinitialized, dockWidget, &QObject::deleteLater);
    connect(IODevice, &TIODeviceModel::showRequested, dockWidget, &TDockWidget::show);
    m_dockManager->addCenterDockWidgetTab(dockWidget, m_welcomeDockWidget);
}

void TMainWindow::createScopeDockWidget(TScopeModel * scope)
{
    TScopeWidget * widget = new TScopeWidget(scope);
    QString title = widget->windowTitle();
    TDockWidget * dockWidget = new TDockWidget(title);
    dockWidget->setWidget(widget);
    m_viewMenu->addAction(dockWidget->toggleViewAction());
    connect(scope, &TScopeModel::deinitialized, dockWidget, [=](){ m_viewMenu->removeAction(dockWidget->toggleViewAction()); });
    connect(scope, &TScopeModel::deinitialized, dockWidget, [=](){ m_dockManager->removeDockWidget(dockWidget); });
    connect(scope, &TScopeModel::deinitialized, dockWidget, &QObject::deleteLater);
    connect(scope, &TScopeModel::showRequested, dockWidget, &TDockWidget::show);
    m_dockManager->addCenterDockWidgetTab(dockWidget, m_welcomeDockWidget);
}

void TMainWindow::createAnalDeviceDockWidget(TAnalDeviceModel * analDevice)
{
    TCommunicationDeviceWidget * widget = new TCommunicationDeviceWidget(analDevice, m_projectModel->protocolContainer());
    QString title = widget->windowTitle();
    TDockWidget * dockWidget = new TDockWidget(title);
    dockWidget->setWidget(widget);
    m_viewMenu->addAction(dockWidget->toggleViewAction());
    connect(analDevice, &TAnalDeviceModel::deinitialized, dockWidget, [=](){ m_viewMenu->removeAction(dockWidget->toggleViewAction()); });
    connect(analDevice, &TAnalDeviceModel::deinitialized, dockWidget, [=](){ m_dockManager->removeDockWidget(dockWidget); });
    connect(analDevice, &TAnalDeviceModel::deinitialized, dockWidget, &QObject::deleteLater);
    connect(analDevice, &TAnalDeviceModel::showRequested, dockWidget, &TDockWidget::show);
    m_dockManager->addCenterDockWidgetTab(dockWidget, m_welcomeDockWidget);
}


void TMainWindow::openProtocolEditor(const QString & protocolName)
{
    createProtocolManagerDockWidget();

    bool ok;
    TProtocolWidget * protocolWidget = (TProtocolWidget *)m_protocolManagerDockWidget->widget();
    protocolWidget->openEditor(protocolName, &ok);

    if(!ok) {
        qWarning("Protocol not found, editor widget could not be opened.");
    }
}

void TMainWindow::createProtocolManagerDockWidget()
{
    if(m_protocolManagerDockWidget) {
        m_protocolManagerDockWidget->show();
        return;
    }

    //create dock widget with Protocol Widget
    TProtocolWidget * widget = new TProtocolWidget(m_projectModel->protocolContainer(), this);
    m_protocolManagerDockWidget = new TDockWidget(tr("Protocol manager"), this);
    m_protocolManagerDockWidget->setWidget(widget);

    m_viewMenu->addAction(m_protocolManagerDockWidget->toggleViewAction());
    m_dockManager->addCenterDockWidgetTab(m_protocolManagerDockWidget, m_welcomeDockWidget);

    // show the widget automatically
    m_protocolManagerDockWidget->show();
}

void TMainWindow::createScenarioManagerDockWidget()
{
    if(m_scenarioManagerDockWidget) {
        m_scenarioManagerDockWidget->show();
        return;
    }

    //create dock widget with Scenario Widget
    TScenarioWidget * widget = new TScenarioWidget(m_projectModel->scenarioContainer(), this);
    m_scenarioManagerDockWidget = new TDockWidget(tr("Scenario manager"), this);
    m_scenarioManagerDockWidget->setWidget(widget);

    m_viewMenu->addAction(m_scenarioManagerDockWidget->toggleViewAction());
    m_dockManager->addCenterDockWidgetTab(m_scenarioManagerDockWidget, m_welcomeDockWidget);

    // show the widget automatically
    m_scenarioManagerDockWidget->show();
}

void TMainWindow::createScenarioEditorDockWidget(TScenarioModel * scenario)
{
    //create dock widget with Scenario Editor Widget
    TScenarioEditorWidget * scenarioEditorWidget = new TScenarioEditorWidget(scenario, m_projectModel, this);
    TDockWidget * scenarioEditorDockWidget = new TDockWidget(QString("%1 - %2").arg(tr("Scenario"), scenario->name()), this);
    scenarioEditorDockWidget->setDeleteOnClose(true);
    scenarioEditorDockWidget->setWidget(scenarioEditorWidget);

    connect(scenarioEditorDockWidget, &TDockWidget::closed, scenarioEditorDockWidget, [=](){
        m_viewMenu->removeAction(scenarioEditorDockWidget->toggleViewAction());
        m_scenarioEditorDockWidgets.removeAll(scenarioEditorDockWidget);
        m_dockManager->removeDockWidget(scenarioEditorDockWidget);
    });
    // no need to connect &QObject::deleteLater the scenarioEditorDockWidget, since it has setDeleteOnClose(true)

    m_viewMenu->addAction(scenarioEditorDockWidget->toggleViewAction());
    m_dockManager->addCenterDockWidgetTab(scenarioEditorDockWidget, m_welcomeDockWidget);

    // show the widget automatically
    scenarioEditorDockWidget->show();

    m_scenarioEditorDockWidgets.append(scenarioEditorDockWidget);
}

void TMainWindow::createGraphDockWidget(TGraph * graph)
{
    //create dock widget with Scenario Editor Widget
    TGraphWidget * graphWidget = new TGraphWidget(graph, this);
    TDockWidget * graphDockWidget = new TDockWidget(QString("%1 - %2").arg(tr("Graph"), graph->name()), this);
    graphDockWidget->setDeleteOnClose(true);
    graphDockWidget->setWidget(graphWidget);

    connect(graphDockWidget, &TDockWidget::closed, graphDockWidget, [=](){
        m_viewMenu->removeAction(graphDockWidget->toggleViewAction());
        m_graphDockWidgets.removeAll(graphDockWidget);
        m_dockManager->removeDockWidget(graphDockWidget);
    });
    // no need to connect &QObject::deleteLater the graphDockWidget, since it has setDeleteOnClose(true)

    m_viewMenu->addAction(graphDockWidget->toggleViewAction());
    m_dockManager->addCenterDockWidgetTab(graphDockWidget, m_welcomeDockWidget);

    // show the widget automatically
    graphDockWidget->show();
    graphWidget->drawGraph();

    m_graphDockWidgets.append(graphDockWidget);
}


//void TMainWindow::closeDockWidget(TDockWidget * widget)
//{
//    QList<TDockWidget *> dockWidgets = m_DockManager->dockWidgetsMap().values();

//    for (int i = 0; i < dockWidgets.length(); i++) {
//        if (dockWidgets[i]->widget() == widget) {
//            dockWidgets[i]->setFeature(TDockWidget::DockWidgetFeature::DeleteContentOnClose, true);
//            dockWidgets[i]->closeDockWidget();
//            return;
//        }
//    }
//}

void TMainWindow::showDeviceWizard()
{
    TDeviceWizard * deviceWizard = new TDeviceWizard(m_projectModel->componentContainer(), this);
    deviceWizard->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    deviceWizard->show();
}

void TMainWindow::newProject()
{
    if (m_projectModel)
        closeProject();

    m_projectModel = new TProjectModel(this);

    connect(m_projectModel, &TProjectModel::IODeviceInitialized, this, &TMainWindow::createIODeviceDockWidget);
    connect(m_projectModel, &TProjectModel::scopeInitialized, this, &TMainWindow::createScopeDockWidget);
    connect(m_projectModel, &TProjectModel::analDeviceInitialized, this, &TMainWindow::createAnalDeviceDockWidget);

    createProjectDockWidget(m_projectModel);
}

void TMainWindow::openProject()
{
    QStringList filters;
    filters << "TraceXpert project file (*.txp)"
            << "Any files (*)";

    QFileDialog openDialog;
    openDialog.setNameFilters(filters);
    openDialog.setAcceptMode(QFileDialog::AcceptOpen);
    openDialog.setFileMode(QFileDialog::ExistingFile);
    openDialog.setDirectory(m_projectDirectory);
    if (!openDialog.exec()) return;

    TLoadingDialog * loadingDialog = TLoadingDialog::showDialog(this, "Loading project...");

    if (m_projectModel)
        closeProject();

    m_projectFileName = openDialog.selectedFiles().constFirst();
    m_projectDirectory = openDialog.directory();

    QFile projectFile(m_projectFileName);
    projectFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray documentArray = projectFile.readAll();
    projectFile.close();

    QDomDocument document;
    document.setContent(documentArray);

    QDomElement projectElement = document.documentElement();

    m_projectModel = new TProjectModel(this);

    connect(m_projectModel, &TProjectModel::IODeviceInitialized, this, &TMainWindow::createIODeviceDockWidget);
    connect(m_projectModel, &TProjectModel::scopeInitialized, this, &TMainWindow::createScopeDockWidget);
    connect(m_projectModel, &TProjectModel::analDeviceInitialized, this, &TMainWindow::createAnalDeviceDockWidget);

    try {
        m_projectModel->load(&projectElement);
    }
    catch (QString message) {
        QMessageBox::critical(this, tr("Project parsing failed"), tr("Unable to parse selected project file: %1").arg(message));

        if (m_projectModel) {
            delete m_projectModel;
            m_projectModel = nullptr;
        }

        loadingDialog->closeAndDeleteLater();
        return;
    }

    createProjectDockWidget(m_projectModel);
    loadingDialog->closeAndDeleteLater();
}

void TMainWindow::saveProject(bool saveAs)
{
    QDomDocument document;
    document.appendChild(m_projectModel->save(document));

    if (m_projectFileName.isEmpty() || saveAs) {

        QStringList filters;
        filters << "TraceXpert project file (*.txp)"
                << "Any files (*)";

        QFileDialog saveDialog;
        saveDialog.setNameFilters(filters);
        saveDialog.setAcceptMode(QFileDialog::AcceptSave);
        saveDialog.setFileMode(QFileDialog::AnyFile);
        saveDialog.setDirectory(m_projectDirectory);
        saveDialog.selectFile(m_projectFileName);
        if (!saveDialog.exec()) return;

        m_projectFileName = saveDialog.selectedFiles().constFirst();
        m_projectDirectory = saveDialog.directory();
    }

    QFile projectFile(m_projectFileName);
    projectFile.open(QIODevice::WriteOnly | QIODevice::Text);
    projectFile.write(document.toByteArray());
    projectFile.close();
}

void TMainWindow::saveProjectAs()
{
    saveProject(true);
}

bool TMainWindow::closeProject()
{
    for(TDockWidget * scenarioEditorDockWidget : std::as_const(m_scenarioEditorDockWidgets)) {
        TScenarioEditorWidget * editor = dynamic_cast<TScenarioEditorWidget *>(scenarioEditorDockWidget->widget());

        if (editor && !editor->close())
            return false;
    }

    if (!TDialog::closeConfirmation(this, "project")) {
        return false;
    }

    for(TDockWidget * graphDockWidget : std::as_const(m_graphDockWidgets)) {
        graphDockWidget->close();
    }

    if (m_projectDockWidget) {
        m_viewMenu->removeAction(m_projectDockWidget->toggleViewAction());
        m_dockManager->removeDockWidget(m_projectDockWidget);
        delete m_projectDockWidget;

        m_projectDockWidget = nullptr;
    }

    if (m_protocolManagerDockWidget) {
        m_viewMenu->removeAction(m_protocolManagerDockWidget->toggleViewAction());
        m_dockManager->removeDockWidget(m_protocolManagerDockWidget);
        delete m_protocolManagerDockWidget;

        m_protocolManagerDockWidget = nullptr;
    }

    if(m_scenarioManagerDockWidget) {
        m_viewMenu->removeAction(m_scenarioManagerDockWidget->toggleViewAction());
        m_dockManager->removeDockWidget(m_scenarioManagerDockWidget);
        delete m_scenarioManagerDockWidget;

        m_scenarioManagerDockWidget = nullptr;
    }

    if (m_projectModel) {
        delete m_projectModel;
        m_projectModel = nullptr;
    }

    m_saveProjectAction->setEnabled(false);
    m_saveProjectAsAction->setEnabled(false);
    m_openDeviceAction->setEnabled(false);

    m_projectFileName = QString();

    return true;
}

void TMainWindow::showHelp()
{
    QWidget * helpWindow = new QWidget(this, Qt::Window);

    helpWindow->setWindowTitle(tr("Help"));
    helpWindow->setAttribute(Qt::WA_DeleteOnClose);

    QHelpEngine * helpEngine = new QHelpEngine(QApplication::applicationDirPath() + "/docs/docs.qhc");

    QTabWidget * helpTabWidget = new QTabWidget;
    helpTabWidget->setMaximumWidth(200);
    helpTabWidget->addTab(helpEngine->contentWidget(), "Contents");
    helpTabWidget->addTab(helpEngine->indexWidget(), "Index");

    THelpBrowser * helpBrowser = new THelpBrowser(helpEngine);
    helpBrowser->setSource(QUrl("qthelp://org.example.docs/docs/README.html"));
    connect(helpEngine->contentWidget(), &QHelpContentWidget::linkActivated, helpBrowser, [=](const QUrl & source) { helpBrowser->setSource(source); });
    connect(helpEngine->indexWidget(), &QHelpIndexWidget::documentActivated, helpBrowser, [=](const QHelpLink & document) { helpBrowser->setSource(document.url); });

    QHBoxLayout * layout = new QHBoxLayout;
    layout->addWidget(helpTabWidget);
    layout->addWidget(helpBrowser);

    helpWindow->setLayout(layout);

    TDockWidget * helpWindowDockWidget = new TDockWidget(helpWindow->windowTitle(), this);
    helpWindowDockWidget->setWidget(helpWindow);

    m_dockManager->addCenterDockWidgetTab(helpWindowDockWidget, m_welcomeDockWidget);

    // show the widget automatically
    helpWindowDockWidget->show();
}

void TMainWindow::showAbout()
{
    QMessageBox about(this);

    about.setWindowTitle(tr("About TraceXpert"));
    about.setText(tr("TraceXpert\nVersion: 0.1\n© 2025 Embedded Security Lab\nFaculty of Information Technology\nCzech Technical University in Prague"));
    about.setStandardButtons(QMessageBox::Ok);
    about.setIconPixmap(QPixmap(":/icons/tracexpert512.png").scaled(50,50, Qt::KeepAspectRatio, Qt::SmoothTransformation));   // here is the error
    about.setDefaultButton(QMessageBox::Ok);
    about.show();
    about.exec();
}

void TMainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();

    if (m_projectModel && !closeProject())
        event->ignore();
    else
        event->accept();
}

void TMainWindow::writeSettings()
{
    //QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Czech Technical University", "traceXpert");
    QSettings settings(QString("traceXpert.ini"), QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
    settings.setValue("projectDirectory", m_projectDirectory.absolutePath());
    settings.endGroup();
}

void TMainWindow::readSettings()
{
    //QSettings settings(QSettings::IniFormat, QSettings::UserScope, "Czech Technical University", "traceXpert");
    QSettings settings(QString("traceXpert.ini"), QSettings::IniFormat);

    settings.beginGroup("MainWindow");
    restoreState(settings.value("windowState").toByteArray());
    restoreGeometry(settings.value("geometry").toByteArray());
    m_projectDirectory = QDir(settings.value("projectDirectory").toString());
    settings.endGroup();
}
