#include <QMenu>
#include <QMenuBar>
#include <QFile>
#include <QMessageBox>

#include "tmainwindow.h"
#include "qfiledialog.h"
#include "tdevicewizard.h"
#include "tiodevicewidget.h"
#include "tscopewidget.h"
#include "tprojectview.h"
#include "tprojectmodel.h"
#include "protocol/tprotocolwidget.h"

TMainWindow::TMainWindow(QWidget * parent)
    : QMainWindow(parent)
{
    setCentralWidget(nullptr);

    createActions();
    createMenus();

    m_dockManager = TDockManagerInstance;

    //create dock widget with Project Widget
    TDockWidget * projectDockWidget = new TDockWidget(tr("Project"), this);
    m_projectView = new TProjectView(this);
    m_projectModel = nullptr;
    m_projectDirectory = QDir::current();
    m_projectView->setModel(m_projectModel);
    projectDockWidget->setWidget(m_projectView);
    m_dockManager->addDockWidget(TDockArea::LeftDockWidgetArea, projectDockWidget);
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
    connect(m_closeProjectAction, SIGNAL(triggered()), this, SLOT(closeProject()));

    m_openDeviceAction = new QAction(tr("Open device"), this);
    m_openDeviceAction->setStatusTip(tr("Open a device using device wizard"));
    connect(m_openDeviceAction, SIGNAL(triggered()), this, SLOT(showDeviceWizard()));
}

void TMainWindow::createIODeviceDockWidget(TIODeviceModel * IODevice)
{
    TIODeviceWidget * widget = new TIODeviceWidget(IODevice, m_projectModel->protocolContainer());
    QString title = widget->windowTitle();
    TDockWidget * dockWidget = new TDockWidget(title);
    dockWidget->setWidget(widget);
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
    connect(scope, &TScopeModel::deinitialized, dockWidget, &TDockWidget::close);
    connect(scope, &TScopeModel::showRequested, dockWidget, &TDockWidget::show);
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, dockWidget);
}

void TMainWindow::openProtocolManagerWidget()
{
    //create dock widget with Protocol Widget
    TProtocolWidget * widget = new TProtocolWidget(m_projectModel->protocolContainer(), this);
    TDockWidget * dockWidget = new TDockWidget(tr("Protocol manager"), this);
    dockWidget->setWidget(widget);
    m_dockManager->addDockWidget(TDockArea::RightDockWidgetArea, dockWidget);
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
    m_projectView->setModel(m_projectModel);
    connect(m_projectModel, &TProjectModel::IODeviceInitialized, this, &TMainWindow::createIODeviceDockWidget);
    connect(m_projectModel, &TProjectModel::scopeInitialized, this, &TMainWindow::createScopeDockWidget);

    m_saveProjectAction->setEnabled(true);
    m_saveProjectAsAction->setEnabled(true);
}

void TMainWindow::openProject()
{
    if (m_projectModel)
        closeProject();

    QStringList filters;
    filters << "TraceXpert project file (*.txp)"
            << "Any files (*)";

    QFileDialog openDialog;
    openDialog.setNameFilters(filters);
    openDialog.setAcceptMode(QFileDialog::AcceptOpen);
    openDialog.setFileMode(QFileDialog::ExistingFile);
    openDialog.setDirectory(m_projectDirectory);
    if (!openDialog.exec()) return;

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

    try {
        m_projectModel->load(&projectElement);
    }
    catch (QString message) {
        QMessageBox::critical(this, tr("Project parsing failed"), tr("Unable to parse selected project file: %1").arg(message));
        return;
    }

    m_projectView->setModel(m_projectModel);

    m_saveProjectAction->setEnabled(true);
    m_saveProjectAsAction->setEnabled(true);
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
    m_projectView->setModel(nullptr);

    if (m_projectModel) {
        delete m_projectModel;
        m_projectModel = nullptr;
    }

    m_saveProjectAction->setEnabled(false);
    m_saveProjectAsAction->setEnabled(false);

    m_projectFileName = QString();
}
