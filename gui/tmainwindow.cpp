#include <QDockWidget>
#include <QLabel>
#include "tmainwindow.h"

#define USE_ADS

TMainWindow::TMainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setCentralWidget(nullptr);

#ifdef USE_ADS
    m_DockManager = new ads::CDockManager(this);

    // create dock widget with Oscilloscope Widget
    ads::CDockWidget * oscilloscopeDockWidget = new ads::CDockWidget(tr("Oscilloscope"), this);
    QLabel * oscilloscopeWidget = new QLabel("This is an Oscilloscope widget", this);
    oscilloscopeDockWidget->setWidget(oscilloscopeWidget);
    m_DockManager->addDockWidget(ads::LeftDockWidgetArea, oscilloscopeDockWidget);

    // create dock widget with IO Device Widget
    ads::CDockWidget * IODeviceDockWidget = new ads::CDockWidget(tr("IO device"), this);
    QLabel * IODeviceWidget = new QLabel("This is an IO device widget", this);
    IODeviceDockWidget->setWidget(IODeviceWidget);
    m_DockManager->addDockWidget(ads::RightDockWidgetArea, IODeviceDockWidget);

    // create dock widget with Protocol Widget
    ads::CDockWidget * ProtocolDockWidget = new ads::CDockWidget(tr("Protocol"), this);
    QLabel * ProtocolWidget = new QLabel("This is a Protocol widget", this);
    ProtocolDockWidget->setWidget(ProtocolWidget);
    m_DockManager->addDockWidget(ads::TopDockWidgetArea, ProtocolDockWidget);

    // create dock widget with Scenario Widget
    ads::CDockWidget * ScenarioDockWidget = new ads::CDockWidget(tr("Scenario"), this);
    QLabel * ScenarioWidget = new QLabel("This is a Scenario widget", this);
    ScenarioDockWidget->setWidget(ScenarioWidget);
    m_DockManager->addDockWidget(ads::BottomDockWidgetArea, ScenarioDockWidget);
#else

    // create dock widget with Oscilloscope Widget
    QDockWidget * oscilloscopeDockWidget = new QDockWidget(tr("Oscilloscope"), this);
    QLabel * oscilloscopeWidget = new QLabel("This is an Oscilloscope widget", this);
    oscilloscopeDockWidget->setWidget(oscilloscopeWidget);
    addDockWidget(Qt::LeftDockWidgetArea, oscilloscopeDockWidget);

    // create dock widget with IO Device Widget
    QDockWidget * IODeviceDockWidget = new QDockWidget(tr("IO device"), this);
    QLabel * IODeviceWidget = new QLabel("This is an IO device widget", this);
    IODeviceDockWidget->setWidget(IODeviceWidget);
    addDockWidget(Qt::RightDockWidgetArea, IODeviceDockWidget);

    // create dock widget with Protocol Widget
    QDockWidget * ProtocolDockWidget = new QDockWidget(tr("Protocol"), this);
    QLabel * ProtocolWidget = new QLabel("This is a Protocol widget", this);
    ProtocolDockWidget->setWidget(ProtocolWidget);
    addDockWidget(Qt::TopDockWidgetArea, ProtocolDockWidget);

    // create dock widget with Scenario Widget
    QDockWidget * ScenarioDockWidget = new QDockWidget(tr("Scenario"), this);
    QLabel * ScenarioWidget = new QLabel("This is a Scenario widget", this);
    ScenarioDockWidget->setWidget(ScenarioWidget);
    addDockWidget(Qt::BottomDockWidgetArea, ScenarioDockWidget);
#endif
}

TMainWindow::~TMainWindow()
{
}

