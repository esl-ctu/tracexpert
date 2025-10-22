#include "tdockmanager.h"
#include <qevent.h>

TDockWidget::TDockWidget(const QString & title, QWidget * parent)
    : TDockWidgetBase(title, parent)
{

}

#ifdef USE_ADS
void TDockWidget::setWidget(QWidget* widget, eInsertMode InsertMode) {

    if(this->widget()) {
        disconnect(this, &TDockWidget::closeRequested, this->widget(), nullptr);
    }

    CDockWidget::setWidget(widget, InsertMode);

    connect(this, &TDockWidget::closeRequested, this->widget(), [=](){
        if(this->widget()->close()) {
            this->closeDockWidgetInternal(true);
        }
    });
}
#endif

#ifndef USE_ADS
bool TDockWidget::isClosed() {
    return isHidden();
}
#endif

void TDockWidget::setDeleteOnClose(bool value) {
    #ifdef USE_ADS
    setFeature(TDockWidget::DockWidgetFeature::CustomCloseHandling, value);
    setFeature(TDockWidget::DockWidgetFeature::DeleteContentOnClose, value);
    #else
    setAttribute(Qt::WA_DeleteOnClose);
    #endif
}


void TDockWidget::show()
{
    if (!toggleViewAction()->isChecked()) {
        toggleViewAction()->trigger();
    };
    raise();
}

void TDockWidget::close()
{
#ifdef USE_ADS    
    closeDockWidget();
#else
    ((QDockWidget *)this)->hide();
#endif
}

#ifndef USE_ADS
void TDockWidget::closeEvent(QCloseEvent *event)
{
    if(this->widget()) {
        if(this->widget()->close()) {
            emit closed();
        }
        else {
            event->ignore();
        }
    }
}
#endif

TDockManager::TDockManager(QWidget *parent, Qt::WindowFlags flags)
#ifdef USE_ADS
    : TDockManagerBase(parent)
#else
    : m_mainWindow((TDockManagerBase *)parent)
#endif
{

}

#ifndef USE_ADS
void TDockManager::addDockWidget(TDockArea dockArea, TDockWidget * dockWidget) {
    m_mainWindow->addDockWidget(dockArea, dockWidget);
}

void TDockManager::removeDockWidget(TDockWidget * dockWidget) {
    m_mainWindow->removeDockWidget(dockWidget);
}
#endif

/**
 * Adds a widget to the center dock as a tab.
 * It does NOT show the tab, call show() on the widget to show it to the user.
 */
void TDockManager::addCenterDockWidgetTab(TDockWidget * dockWidget, TDockWidget * existingDockWidget) {
#ifdef USE_ADS
    // prevent newly added widget showing on top/taking focus automatically

    // find the currently open tab
    ads::CDockWidget * currentTab = nullptr;
    for(ads::CDockWidget * widget : dockWidgets()) {
        if(widget->isCurrentTab() && widget->dockAreaWidget() == existingDockWidget->dockAreaWidget()) {
            currentTab = widget;
        }
    }

    addDockWidget(TDockArea::CenterDockWidgetArea, dockWidget, existingDockWidget->dockAreaWidget());

    // set the previously opened tab as current again
    if(currentTab) {
        currentTab->setAsCurrentTab();
    }
#else
    m_mainWindow->tabifyDockWidget(existingDockWidget, dockWidget);
#endif
}

void TDockManager::addCenterDockWidget(TDockWidget * dockWidget) {
#ifdef USE_ADS
    dockWidget->setFeature(ads::CDockWidget::DockWidgetMovable, false);
    dockWidget->setFeature(ads::CDockWidget::DockWidgetFloatable, false);
    dockWidget->setFeature(ads::CDockWidget::DockWidgetClosable, false);
    addDockWidget(TDockArea::CenterDockWidgetArea, dockWidget);
#else
    m_mainWindow->addDockWidget(TDockArea::RightDockWidgetArea, dockWidget);
#endif
}



