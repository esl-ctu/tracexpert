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
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    menuBar()->addMenu(fileMenu);

    QMenu *devicesMenu = new QMenu(tr("Devices"), this);
    devicesMenu->addAction(m_openDeviceAction);
    menuBar()->addMenu(devicesMenu);
}

void TMainWindow::createActions()
{
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
