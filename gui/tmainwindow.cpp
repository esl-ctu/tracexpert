#include <QMenu>
#include <QMenuBar>

#include "tmainwindow.h"
#include "tdevicewizard.h"
#include "tiodevicewidget.h"
#include "tscopewidget.h"
#include "tprojectview.h"
#include "tprojectmodel.h"

TMainWindow::TMainWindow(QWidget * parent)
    : QMainWindow(parent)
{
    setCentralWidget(nullptr);

    createActions();
    createMenus();

    m_dockManager = TDockManagerInstance;

    //create dock widget with Project Widget
    TDockWidget * projectDockWidget = new TDockWidget(tr("Project"), this);
    TProjectView * projectView = new TProjectView(this);
    m_projectModel = new TProjectModel(this);
    connect(m_projectModel, &TProjectModel::IODeviceInitialized, this, &TMainWindow::createIODeviceDockWidget);
    connect(m_projectModel, &TProjectModel::scopeInitialized, this, &TMainWindow::createScopeDockWidget);
    projectView->setModel(m_projectModel);
    projectDockWidget->setWidget(projectView);
    m_dockManager->addDockWidget(TDockArea::LeftDockWidgetArea, projectDockWidget);

    //projectWidget->refresh();
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
    //connect(m_newProjectAction, SIGNAL(triggered()), this, SLOT(newFile()));

    m_openProjectAction = new QAction(tr("&Open project"), this);
    m_openProjectAction->setShortcuts(QKeySequence::Open);
    m_openProjectAction->setStatusTip(tr("Open an existing project"));
    m_newProjectAction->setIcon(QIcon::fromTheme("document-open"));
    //connect(m_openProjectAction, SIGNAL(triggered()), this, SLOT(openFile()));

    m_saveProjectAction = new QAction(tr("&Save project"), this);
    m_saveProjectAction->setShortcuts(QKeySequence::Save);
    m_newProjectAction->setIcon(QIcon::fromTheme("document-save"));
    m_saveProjectAction->setStatusTip(tr("Save the currect project"));
    //connect(m_saveProjectAction, SIGNAL(triggered()), this, SLOT(saveFile()));

    m_saveProjectAsAction = new QAction(tr("&Save project as"), this);
    m_saveProjectAsAction->setShortcuts(QKeySequence::SaveAs);
    m_newProjectAction->setIcon(QIcon::fromTheme("document-save-as"));
    m_saveProjectAsAction->setStatusTip(tr("Select path and save the current project"));
    //connect(m_saveProjectAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

    m_openDeviceAction = new QAction(tr("Open device"), this);
    m_openDeviceAction->setStatusTip(tr("Open a device using device wizard"));
    connect(m_openDeviceAction, SIGNAL(triggered()), this, SLOT(showDeviceWizard()));
}

void TMainWindow::createIODeviceDockWidget(TIODeviceModel * IODevice)
{
    TIODeviceWidget * widget = new TIODeviceWidget(IODevice);
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
