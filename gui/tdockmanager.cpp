#include "tdockmanager.h"

TDockWidget::TDockWidget(const QString & title, QWidget * parent)
    : TDockWidgetBase(title, parent)
{
    #ifdef USE_ADS
    setFeature(TDockWidget::DockWidgetFeature::CustomCloseHandling, true);
    setFeature(TDockWidget::DockWidgetFeature::DeleteContentOnClose, true);
    #endif
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
#endif
}
