#include "tdockmanager.h"

TDockWidget::TDockWidget(const QString & title, QWidget * parent)
    : TDockWidgetBase(title, parent)
{
    
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
    setFeature(TDockWidget::DockWidgetFeature::DeleteContentOnClose, true);
    closeDockWidget();
#endif
}
