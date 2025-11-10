#ifndef TGRAPHWIDGET_H
#define TGRAPHWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

#include "tgraph.h"
#include "widgets/tconfigparamwidget.h"

class TGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TGraphWidget(TGraph * graph, QWidget * parent = nullptr);

private slots:
    bool applyParams();

private:
    TGraph * m_graph;

    TConfigParamWidget * m_paramWidget;
};

#endif // TGRAPHWIDGET_H
