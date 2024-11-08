#include <QMenu>
#include <QMenuBar>
#include <QFile>
#include <QMessageBox>
#include <QSettings>

#include "tmainwindow.h"
#include "qfiledialog.h"
#include "tdevicewizard.h"
#include "tiodevicewidget.h"
#include "tanaldevicewidget.h"
#include "scenario/tscenariowidget.h"
#include "scenario/tscenarioeditorwidget.h"
#include "tscopewidget.h"
#include "tprojectview.h"
#include "tprojectmodel.h"
#include "protocol/tprotocolwidget.h"

TMainWindow::TMainWindow(QWidget * parent)
    : QMainWindow(parent)
{
    setCentralWidget(nullptr);

    m_dockManager = TDockManagerInstance;
    m_projectModel = nullptr;
    m_projectView = nullptr;

    m_protocolManagerDockWidget = nullptr;
    m_projectDockWidget = nullptr;
    m_scenarioManagerDockWidget = nullptr;

    createActions();
    createMenus();

    readSettings();

    //createScenarioEditorWidget();
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
}

void TMainWindow::createProjectDockWidget(TProjectModel * model)
{
    m_projectDockWidget = new TDockWidget(tr("Project"), this);
    m_projectView = new TProjectView(this);
    m_projectModel = model;
    m_projectDirectory = QDir::current();
    m_projectView->setModel(m_projectModel);
    m_projectView->expandAll();
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
    TIODeviceWidget * widget = new TIODeviceWidget(IODevice, m_projectModel->protocolContainer());
    QString title = widget->windowTitle();
    TDockWidget * dockWidget = new TDockWidget(title);
    dockWidget->setWidget(widget);
    m_viewMenu->addAction(dockWidget->toggleViewAction());
    connect(IODevice, &TIODeviceModel::deinitialized, dockWidget, [=](){ m_viewMenu->removeAction(dockWidget->toggleViewAction()); });
    connect(IODevice, &TIODeviceModel::deinitialized, dockWidget, &TDockWidget::close);
    connect(IODevice, &TIODeviceModel::showRequested, dockWidget, &TDockWidget::show);
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, dockWidget);
}

void TMainWindow::createScopeDockWidget(TScopeModel * scope)
{
    TScopeWidget * widget = new TScopeWidget(scope);
    QString title = widget->windowTitle();
    TDockWidget * dockWidget = new TDockWidget(title);
    dockWidget->setWidget(widget);
    m_viewMenu->addAction(dockWidget->toggleViewAction());
    connect(scope, &TScopeModel::deinitialized, dockWidget, [=](){ m_viewMenu->removeAction(dockWidget->toggleViewAction()); });
    connect(scope, &TScopeModel::deinitialized, dockWidget, &TDockWidget::close);
    connect(scope, &TScopeModel::showRequested, dockWidget, &TDockWidget::show);
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, dockWidget);
}

void TMainWindow::createAnalDeviceDockWidget(TAnalDeviceModel * analDevice)
{
    TAnalDeviceWidget * widget = new TAnalDeviceWidget(analDevice, m_projectModel->protocolContainer());
    QString title = widget->windowTitle();
    TDockWidget * dockWidget = new TDockWidget(title);
    dockWidget->setWidget(widget);
    m_viewMenu->addAction(dockWidget->toggleViewAction());
    connect(analDevice, &TAnalDeviceModel::deinitialized, dockWidget, [=](){ m_viewMenu->removeAction(dockWidget->toggleViewAction()); });
    connect(analDevice, &TAnalDeviceModel::deinitialized, dockWidget, &TDockWidget::close);
    connect(analDevice, &TAnalDeviceModel::showRequested, dockWidget, &TDockWidget::show);
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, dockWidget);
}


void TMainWindow::openProtocolEditor(const QString & protocolName)
{
    createProtocolManagerWidget();

    bool ok;
    TProtocolWidget * protocolWidget = (TProtocolWidget *)m_protocolManagerDockWidget->widget();
    protocolWidget->openEditor(protocolName, &ok);

    if(!ok) {
        // TODO TDialog:: protocol not found
        return;
    }
}

void TMainWindow::createProtocolManagerWidget()
{
    if(m_protocolManagerDockWidget) {
        if(!m_protocolManagerDockWidget->isClosed()) {
            m_protocolManagerDockWidget->focusWidget();
            return;
        }
        else {
            m_viewMenu->removeAction(m_protocolManagerDockWidget->toggleViewAction());
            delete m_protocolManagerDockWidget;
        }
    }

    //create dock widget with Protocol Widget
    TProtocolWidget * widget = new TProtocolWidget(m_projectModel->protocolContainer(), this);
    m_protocolManagerDockWidget = new TDockWidget(tr("Protocol manager"), this);
    m_protocolManagerDockWidget->setWidget(widget);

    m_viewMenu->addAction(m_protocolManagerDockWidget->toggleViewAction());
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, m_protocolManagerDockWidget);
}

void TMainWindow::openScenarioEditor(TScenarioModel * scenario)
{
    //create dock widget with Scenario Editor Widget
    TScenarioEditorWidget * scenarioEditorWidget = new TScenarioEditorWidget(scenario, m_projectModel, this);
    TDockWidget * scenarioEditorDockWidget = new TDockWidget(QString("%1 - %2").arg(tr("Scenario"), scenario->name()), this);
    scenarioEditorDockWidget->setWidget(scenarioEditorWidget);

    connect(scenarioEditorDockWidget, &TDockWidget::closed, scenarioEditorDockWidget, [=](){
        m_viewMenu->removeAction(scenarioEditorDockWidget->toggleViewAction());
        m_scenarioEditorDockWidgets.removeAll(scenarioEditorDockWidget);
        m_dockManager->removeDockWidget(scenarioEditorDockWidget);
    });
    connect(scenarioEditorDockWidget, &TDockWidget::closed, scenarioEditorDockWidget, &QObject::deleteLater);

    // TODO solve closing confirmation
    // https://docs.ros.org/en/noetic/api/plotjuggler/html/classads_1_1CDockWidget.html#a99fde29fcad39c7f9328ab95bb55f4b2
    //connect(scenarioEditorWidget, &TDockWidget::c, scenarioEditorWidget, &TDockWidget::closeEvent);

    m_viewMenu->addAction(scenarioEditorDockWidget->toggleViewAction());
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, scenarioEditorDockWidget);

    m_scenarioEditorDockWidgets.append(scenarioEditorDockWidget);
}


void TMainWindow::createScenarioManagerWidget()
{
    if(m_scenarioManagerDockWidget) {
        if(!m_scenarioManagerDockWidget->isClosed()) {
            m_scenarioManagerDockWidget->focusWidget();
            return;
        }
        else {
            m_viewMenu->removeAction(m_scenarioManagerDockWidget->toggleViewAction());
            delete m_scenarioManagerDockWidget;
        }
    }

    //create dock widget with Scenario Widget
    TScenarioWidget * widget = new TScenarioWidget(m_projectModel->scenarioContainer(), this);
    m_scenarioManagerDockWidget = new TDockWidget(tr("Scenario manager"), this);
    m_scenarioManagerDockWidget->setWidget(widget);

    m_viewMenu->addAction(m_scenarioManagerDockWidget->toggleViewAction());
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, m_scenarioManagerDockWidget);
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
    createProtocolManagerWidget();
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

        return;
    }

    createProjectDockWidget(m_projectModel);
    createProtocolManagerWidget();
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

void TMainWindow::closeProject()
{
    if (m_projectModel) {
        delete m_projectModel;
        m_projectModel = nullptr;
    }

    if (m_projectDockWidget) {
        m_viewMenu->removeAction(m_projectDockWidget->toggleViewAction());
        m_projectDockWidget->close();
    }

    if (m_protocolManagerDockWidget) {
        m_viewMenu->removeAction(m_protocolManagerDockWidget->toggleViewAction());
        m_protocolManagerDockWidget->close();
    }

    if(m_scenarioManagerDockWidget) {
        m_viewMenu->removeAction(m_scenarioManagerDockWidget->toggleViewAction());
        m_scenarioManagerDockWidget->close();
    }

    for(TDockWidget * scenarioEditorDockWidget : m_scenarioEditorDockWidgets) {
        m_viewMenu->removeAction(scenarioEditorDockWidget->toggleViewAction());
        scenarioEditorDockWidget->close();
    }

    m_saveProjectAction->setEnabled(false);
    m_saveProjectAsAction->setEnabled(false);
    m_openDeviceAction->setEnabled(false);

    m_projectFileName = QString();
}

void TMainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();
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
