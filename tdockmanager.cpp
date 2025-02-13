#include "tdockmanager.h"

TDockWidget::TDockWidget(const QString & title, QWidget * parent)
    : TDockWidgetBase(title, parent)
{

}


void TDockWidget::setWidget(QWidget* widget, eInsertMode InsertMode) {
    #ifdef USE_ADS
    if(this->widget()) {
        disconnect(this, &TDockWidget::closeRequested, this->widget(), nullptr);
    }

    CDockWidget::setWidget(widget, InsertMode);

    connect(this, &TDockWidget::closeRequested, this->widget(), [=](){
        if(this->widget()->close()) {
            this->closeDockWidgetInternal(true);
        }
    });
    #else
    qWarning("setWidget was called, but related features are not implemented");
    #endif
}


void TDockWidget::setDeleteOnClose(bool value) {
    #ifdef USE_ADS
    setFeature(TDockWidget::DockWidgetFeature::CustomCloseHandling, value);
    setFeature(TDockWidget::DockWidgetFeature::DeleteContentOnClose, value);
    #else
    qWarning("setDeleteOnClose was called, but feature is not implemented; memory leaks will occur");
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
#endif
}
