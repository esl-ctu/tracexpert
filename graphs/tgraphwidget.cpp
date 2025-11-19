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
#include "../tdialog.h"

TGraphWidget::TGraphWidget(TGraph * graph, QWidget * parent) : QWidget(parent), m_graph(graph) {
    setWindowTitle(tr("Graph widget - %1").arg(m_graph->name()));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(700, 300);

    connect(m_graph, &TGraph::interpretationChanged, this, &TGraphWidget::interpretationChanged);

    m_graphParamWidget = new TConfigParamWidget(m_graph->graphParams());
    m_graphParamWidget->setMinimumWidth(320);

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TGraphWidget::applyParams);

    m_interpretationParamWidget = new TConfigParamWidget(m_graph->interpretationParams());
    m_interpretationParamWidget->setMinimumWidth(320);

    QVBoxLayout * paramLayout = new QVBoxLayout();
    paramLayout->addWidget(m_graphParamWidget);
    paramLayout->addWidget(applyButton);
    paramLayout->addWidget(m_interpretationParamWidget);

    QWidget * paramWidget = new QWidget();
    paramWidget->setLayout(paramLayout);

    QSplitter * lowerSplitter = new QSplitter(Qt::Horizontal);
    lowerSplitter->addWidget(m_graph);
    lowerSplitter->setStretchFactor(0, 1);
    lowerSplitter->addWidget(paramWidget);
    lowerSplitter->setStretchFactor(1, 0);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addWidget(lowerSplitter);
    layout->setContentsMargins(0, 0, 0, 0);

    setLayout(layout);
}

void TGraphWidget::drawGraph() {
    TLoadingDialog * loadingDialog = TLoadingDialog::showDialog(this->topLevelWidget(), "Drawing chart...");
    m_graph->drawGraph();
    loadingDialog->closeAndDeleteLater();
}

bool TGraphWidget::applyParams() {
    TConfigParam param = m_graph->setGraphParams(m_graphParamWidget->param());
    m_graphParamWidget->setParam(param);

    if (param.getState(true) == TConfigParam::TState::TError) {
        qWarning("Graph parameters not set due to error state!");
        return false;
    }

    drawGraph();

    return true;
}

void TGraphWidget::interpretationChanged() {
    m_interpretationParamWidget->setParam(m_graph->interpretationParams());
}
