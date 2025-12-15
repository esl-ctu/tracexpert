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

    void drawGraph();

private slots:
    bool applyParams();
    void interpretationChanged();

private:
    void saveGraph();
    void renderGraph(QPaintDevice * paintDevice, uint width, uint height);

    TGraph * m_graph;

    TConfigParamWidget * m_graphParamWidget;
    TConfigParamWidget * m_interpretationParamWidget;
};

#endif // TGRAPHWIDGET_H
