
#ifndef TCONFIGPARAMWIDGETTEST_H
#define TCONFIGPARAMWIDGETTEST_H


#include "tconfigparam.h"
#include "tconfigparamwidget.h"
#include <QMainWindow>

class TConfigParamWidgetTest : public QWidget
{
    Q_OBJECT
public:
    explicit TConfigParamWidgetTest(TConfigParam configParam, QWidget * parent = nullptr);
    TConfigParamWidget * configParamWidget;
signals:

};

#endif // TCONFIGPARAMWIDGETTEST_H
