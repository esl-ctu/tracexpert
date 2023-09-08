#ifndef TOSCILLOSCOPEWIDGET_H
#define TOSCILLOSCOPEWIDGET_H

#include <QWidget>

#include "tscopemodel.h"

class TScopeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TScopeWidget(TScopeModel * scope, QWidget * parent = nullptr);
};

#endif // TOSCILLOSCOPEWIDGET_H
