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
    ((QDockWidget *)this)->close();
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
