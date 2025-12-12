#include <QMenu>
#include <QMenuBar>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QOpenGLWidget>
#include <QStatusBar>
#include <QToolBar>

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
#include "help/thelpwidget.h"
#include "buildinfo.h"
#include "twelcomescreen.h"

TMainWindow::TMainWindow(QWidget * parent)
    : QMainWindow(parent)
{
    // Workaround to prevent window reopening when oscilloscope widget is loaded for the first time
    QOpenGLWidget dummyOpenGLWidget;
    setCentralWidget(&dummyOpenGLWidget);

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
    createWelcome();

    createLog(TLogHandler::logWidget());
    createStatusBar(TLogHandler::logLineWidget());

    readSettings();
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
    fileMenu->addSeparator();
    fileMenu->addAction(m_closeProjectAction);
    fileMenu->addSeparator();
    fileMenu->addAction(m_exitAction);
    menuBar()->addMenu(fileMenu);

    m_viewMenu = new QMenu(tr("&View"), this);
    menuBar()->addMenu(m_viewMenu);

    QMenu * toolsMenu = new QMenu(tr("&Tools"), this);
    toolsMenu->addAction(m_openDeviceAction);
    toolsMenu->addAction(m_openProtocolsAction);
    toolsMenu->addAction(m_openScenariosAction);
    menuBar()->addMenu(toolsMenu);

    QMenu * helpMenu = new QMenu(tr("&Help"), this);

    QString helpPath = QApplication::applicationDirPath() + "/docs/docs.qhc";
    if (QFile::exists(helpPath))
        helpMenu->addAction(m_helpAction);

    helpMenu->addAction(m_licenseAction);
    helpMenu->addAction(m_contributorsAction);
    helpMenu->addAction(m_aboutAction);
    menuBar()->addMenu(helpMenu);
}

void TMainWindow::createActions()
{
    m_newProjectAction = new QAction(tr("&New project"), this);
    m_newProjectAction->setShortcuts(QKeySequence::New);
    m_newProjectAction->setStatusTip(tr("Create a new project"));
    m_newProjectAction->setIcon(QPixmap(":/icons/new-project.png"));
    m_newProjectAction->setIconVisibleInMenu(true);
    connect(m_newProjectAction, SIGNAL(triggered()), this, SLOT(newProject()));

    m_openProjectAction = new QAction(tr("&Open project"), this);
    m_openProjectAction->setShortcuts(QKeySequence::Open);
    m_openProjectAction->setStatusTip(tr("Open an existing project"));
    m_openProjectAction->setIcon(QPixmap(":/icons/open-project.png"));
    connect(m_openProjectAction, SIGNAL(triggered()), this, SLOT(openProject()));

    m_saveProjectAction = new QAction(tr("&Save project"), this);
    m_saveProjectAction->setShortcuts(QKeySequence::Save);
    m_saveProjectAction->setIcon(QPixmap(":/icons/save.png"));
    m_saveProjectAction->setStatusTip(tr("Save the currect project"));
    m_saveProjectAction->setEnabled(false);
    connect(m_saveProjectAction, SIGNAL(triggered()), this, SLOT(saveProject()));

    m_saveProjectAsAction = new QAction(tr("&Save project as"), this);
    m_saveProjectAsAction->setShortcuts(QKeySequence::SaveAs);
    m_saveProjectAsAction->setIcon(QPixmap(":/icons/save-as.png"));
    m_saveProjectAsAction->setStatusTip(tr("Select path and save the current project"));
    m_saveProjectAsAction->setEnabled(false);
    connect(m_saveProjectAsAction, SIGNAL(triggered()), this, SLOT(saveProjectAs()));

    m_closeProjectAction = new QAction(tr("&Close project"), this);
    m_closeProjectAction->setShortcuts(QKeySequence::Close);
    m_closeProjectAction->setIcon(QPixmap(":/icons/close-project.png"));
    m_closeProjectAction->setStatusTip(tr("Close the current project"));
    m_closeProjectAction->setEnabled(false);
    connect(m_closeProjectAction, SIGNAL(triggered()), this, SLOT(closeProject()));

    m_exitAction = new QAction(tr("Exit"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    m_exitAction->setStatusTip(tr("Exit the application"));
    m_exitAction->setEnabled(true);
    connect(m_exitAction, &QAction::triggered, this, &QWidget::close);

    m_openDeviceAction = new QAction(tr("Add/open device"), this);
    m_openDeviceAction->setStatusTip(tr("Open a device using device wizard"));
    m_openDeviceAction->setIcon(QPixmap(":/icons/add-open-device.png"));
    m_openDeviceAction->setEnabled(false);
    connect(m_openDeviceAction, SIGNAL(triggered()), this, SLOT(showDeviceWizard()));

    m_openProtocolsAction = new QAction(tr("Protocol manager"), this);
    m_openProtocolsAction->setStatusTip(tr("Open a protocol manager"));
    m_openProtocolsAction->setIcon(QPixmap(":/icons/protocol-manager.png"));
    m_openProtocolsAction->setEnabled(false);
    connect(m_openProtocolsAction, SIGNAL(triggered()), this, SLOT(createProtocolManagerDockWidget()));

    m_openScenariosAction = new QAction(tr("Scenario manager"), this);
    m_openScenariosAction->setStatusTip(tr("Open a scenario manager"));
    m_openScenariosAction->setIcon(QPixmap(":/icons/scenario-manager.png"));
    m_openScenariosAction->setEnabled(false);
    connect(m_openScenariosAction, SIGNAL(triggered()), this, SLOT(createScenarioManagerDockWidget()));

    m_helpAction = new QAction(tr("User Guide"), this);
    m_helpAction->setStatusTip(tr("Show help"));
    m_helpAction->setShortcut(QKeySequence(Qt::Key_F1));
    connect(m_helpAction, SIGNAL(triggered()), this, SLOT(showHelp()));

    m_licenseAction = new QAction(tr("License"), this);
    m_licenseAction->setStatusTip(tr("Show license"));
    connect(m_licenseAction, SIGNAL(triggered()), this, SLOT(showLicense()));

    m_contributorsAction = new QAction(tr("Contributors"), this);
    m_contributorsAction->setStatusTip(tr("Show contributors"));
    connect(m_contributorsAction, SIGNAL(triggered()), this, SLOT(showContributors()));

    m_aboutAction = new QAction(tr("About"), this);
    m_aboutAction->setStatusTip(tr("Show information about this program"));
    connect(m_aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
}

void TMainWindow::createWelcome() {
    m_welcomeDockWidget = new TDockWidget(tr("Welcome"));
/*
    QLabel * imageLabel = new QLabel(this);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setPixmap(QPixmap(":/icons/tracexpert512.png"));*/

    TWelcomeScreen * welcomeScreen = new TWelcomeScreen(this);
    welcomeScreen->setActions(m_newProjectAction, m_openProjectAction, m_saveProjectAction, m_closeProjectAction, m_openDeviceAction, m_openProtocolsAction, m_openScenariosAction);

    m_welcomeDockWidget->setWidget(welcomeScreen);

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
    QToolBar * projectToolbar = new QToolBar(this);

    projectToolbar->setFixedHeight(32);
    projectToolbar->setMovable(false);
    projectToolbar->setFloatable(false);
    projectToolbar->setOrientation(Qt::Horizontal);

    // Set toolbar style
    //projectToolbar->setStyleSheet("QToolBar { border: 0; background: red; padding: 0; margin: 0 }");

    m_projectView = new TProjectView(projectToolbar, this);
    m_projectView->setMinimumWidth(450);
    m_projectModel = model;
    m_projectDirectory = QDir::current();
    m_projectView->setModel(m_projectModel);
    m_projectView->expandToDepth(1);
    m_projectView->resizeColumnToContents(0);

    QWidget * projectWidget = new QWidget;

    QLayout * layout = new QVBoxLayout;
    layout->addWidget(projectToolbar);
    layout->addWidget(m_projectView);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    projectWidget->setLayout(layout);

    m_projectDockWidget = new TDockWidget(tr("Project"), this);
    m_projectDockWidget->setWidget(projectWidget);
    m_dockManager->addDockWidget(TDockArea::LeftDockWidgetArea, m_projectDockWidget);
    m_viewMenu->insertAction(m_viewMenu->actions().isEmpty() ? nullptr : m_viewMenu->actions().constFirst(), m_projectDockWidget->toggleViewAction());

    m_saveProjectAction->setEnabled(true);
    m_saveProjectAsAction->setEnabled(true);
    m_closeProjectAction->setEnabled(true);
    m_openDeviceAction->setEnabled(true);
    m_openScenariosAction->setEnabled(true);
    m_openProtocolsAction->setEnabled(true);
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

        scenarioEditorDockWidget->close();
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
    m_closeProjectAction->setEnabled(false);
    m_openDeviceAction->setEnabled(false);
    m_openProtocolsAction->setEnabled(false);
    m_openScenariosAction->setEnabled(false);

    m_projectFileName = QString();

    return true;
}

void TMainWindow::showHelp()
{
    THelpWidget * helpWidget = new THelpWidget;
    helpWidget->setAttribute(Qt::WA_DeleteOnClose);

    TDockWidget * helpDockWidget = new TDockWidget(helpWidget->windowTitle(), this);
    helpDockWidget->setWidget(helpWidget);

    m_dockManager->addCenterDockWidgetTab(helpDockWidget, m_welcomeDockWidget);

    helpDockWidget->show();
}

void TMainWindow::showLicense()
{
    QDialog dlg(this);
    dlg.setWindowTitle("License");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QString licenseText;
    QFile file(":/LICENSE");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        licenseText = QString::fromUtf8(file.readAll());
    } else {
        licenseText = "This program is free software: you can redistribute it and/or modify\
it under the terms of the GNU General Public License as published by\
the Free Software Foundation, either version 3 of the License, or\
(at your option) any later version.\
\
This program is distributed in the hope that it will be useful,\
but WITHOUT ANY WARRANTY; without even the implied warranty of\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\
GNU General Public License for more details.\
\
You should have received a copy of the GNU General Public License\
along with this program. If not, see <https://www.gnu.org/licenses/>.";
    }

    QTextBrowser *browser = new QTextBrowser(&dlg);
    browser->setText(licenseText);
    browser->setReadOnly(true);
    browser->setMinimumSize(500, 400);

    layout->addWidget(browser);

    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    layout->addWidget(buttons);

    dlg.exec();
}

void TMainWindow::showContributors()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Contributors");

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QString licenseText;
    QFile file(":/CONTRIBUTORS");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)){
        licenseText = QString::fromUtf8(file.readAll());
    } else {
        licenseText = "CONTRIBUTORS file not found!";
    }

    QTextBrowser *browser = new QTextBrowser(&dlg);
    browser->setText(licenseText);
    browser->setReadOnly(true);
    browser->setMinimumSize(500, 400);

    layout->addWidget(browser);

    QDialogButtonBox *buttons =
        new QDialogButtonBox(QDialogButtonBox::Ok, &dlg);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    layout->addWidget(buttons);

    dlg.exec();
}

void TMainWindow::showAbout()
{
    QMessageBox about(this);

    QString aboutText = tr("<h1>TraceXpert %1</h1>\
\
Based on Qt %2 (%3, %4)<br>\
Built on %5<br>\
Revision %6<br><br>\
\
© %7 Embedded Security Lab (see <i>Help > Contributors</i>)<br>\
Faculty of Information Technology<br>\
Czech Technical University in Prague<br><br>\
\
This program is distributed in the hope that it will be useful,<br>\
but WITHOUT ANY WARRANTY; without even the implied warranty of<br>\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.<br>\
See the GNU General Public License v3.0 (<i>Help > License</i>) for more details.");

    QString compiler;

    #if defined(__clang__)
        compiler = QString("Clang %1.%2").arg(__clang_major__).arg(__clang_minor__);
    #elif defined(__GNUC__)
        compiler = QString("GCC %1.%2.%3")
                       .arg(__GNUC__).arg(__GNUC_MINOR__).arg(__GNUC_PATCHLEVEL__);
    #elif defined(_MSC_VER)
        compiler = QString("MSVC %1").arg(_MSC_VER);
    #else
        compiler = "Unknown compiler";
    #endif

    about.setWindowTitle(tr("About TraceXpert"));
    about.setTextFormat(Qt::RichText);
    about.setText(aboutText.arg(TRACEXPERT_VERSION).arg(QT_VERSION_STR).arg(compiler).arg(QSysInfo::buildAbi()).arg(BUILD_TIMESTAMP).arg(BUILD_GIT_REVISION).arg(COPYRIGHT_YEAR));
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
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        resize(QSize(1360, 800)); // assure minimum size on first launch
    }
    m_projectDirectory = QDir(settings.value("projectDirectory").toString());
    settings.endGroup();
}
