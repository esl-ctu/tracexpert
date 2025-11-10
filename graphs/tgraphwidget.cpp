#include <QLayout>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QSplitter>

#include "tgraphwidget.h"
#include "widgets/tconfigparamwidget.h"

TGraphWidget::TGraphWidget(TGraph * graph, QWidget * parent) : QWidget(parent), m_graph(graph) {
    setWindowTitle(tr("Graph widget - %1").arg(m_graph->name()));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(700, 300);

    QChartView * chartView = new QChartView(m_graph);
    chartView->setRenderHint(QPainter::Antialiasing);

    m_paramWidget = new TConfigParamWidget(m_graph->params());
    m_paramWidget->setMinimumWidth(320);

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TGraphWidget::applyParams);

    QVBoxLayout * paramLayout = new QVBoxLayout();
    paramLayout->addWidget(m_paramWidget);
    paramLayout->addWidget(applyButton);

    QWidget * paramWidget = new QWidget();
    paramWidget->setLayout(paramLayout);

    QSplitter * lowerSplitter = new QSplitter(Qt::Horizontal);
    lowerSplitter->addWidget(chartView);
    lowerSplitter->setStretchFactor(0, 1);
    lowerSplitter->addWidget(paramWidget);
    lowerSplitter->setStretchFactor(1, 0);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(lowerSplitter);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);
}

bool TGraphWidget::applyParams() {
    TConfigParam param = m_graph->setParams(m_paramWidget->param());
    m_paramWidget->setParam(param);

    if (param.getState(true) == TConfigParam::TState::TError) {
        qWarning("Graph parameters not set due to error state!");
        return false;
    }

    return true;
}

