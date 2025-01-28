#ifndef TDOCKMANAGER_H
#define TDOCKMANAGER_H

#define USE_ADS

#ifdef USE_ADS

#include "DockManager.h"

typedef ads::CDockManager TDockManager;
typedef ads::CDockWidget TDockWidgetBase;
#define TDockManagerInstance new TDockManager(this)
#define TDockArea ads::DockWidgetArea

#else

#include <QDockWidget>

typedef QMainWindow TDockManager;
typedef QDockWidget TDockWidgetBase;
#define TDockManagerInstance this
#define TDockArea Qt::DockWidgetArea

#endif

class TDockWidget : public TDockWidgetBase
{
    Q_OBJECT

public:
    explicit TDockWidget(const QString & title, QWidget * parent = nullptr);

    void setDeleteOnClose(bool value);
    void setWidget(QWidget* widget, eInsertMode InsertMode = AutoScrollArea);

public slots:
    void show();
    void close();

};

#endif // TDOCKMANAGER_H
