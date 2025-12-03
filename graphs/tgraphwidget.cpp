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
#include <QFileDialog>
#include <QToolBar>
#include <QSvgGenerator>
#include <QApplication>

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
    paramLayout->setContentsMargins(0, 0, 0, 0);

    QWidget * paramWidget = new QWidget();
    paramWidget->setLayout(paramLayout);

    QSplitter * verticalSplitter = new QSplitter(Qt::Vertical);
    verticalSplitter->addWidget(paramWidget);
    verticalSplitter->addWidget(m_interpretationParamWidget);

    QSplitter * horizontalSplitter = new QSplitter(Qt::Horizontal);
    horizontalSplitter->addWidget(m_graph);
    horizontalSplitter->setStretchFactor(0, 1);
    horizontalSplitter->addWidget(verticalSplitter);
    horizontalSplitter->setStretchFactor(1, 0);

    QAction * saveAction = new QAction(QIcon(":/icons/save.png"), tr("&Save graph to file"), this);
    saveAction->setShortcut(tr("Ctrl+S"));
    saveAction->setStatusTip(tr("Save graph to file"));
    connect(saveAction, &QAction::triggered, this, &TGraphWidget::saveGraph);

    QToolBar * toolBar = new QToolBar();
    toolBar->addAction(saveAction);

    QHBoxLayout * toolbarLayout = new QHBoxLayout;
    toolbarLayout->addWidget(toolBar);

    QVBoxLayout * lowerLayout = new QVBoxLayout();
    lowerLayout->addWidget(horizontalSplitter);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addLayout(toolbarLayout);
    layout->addLayout(lowerLayout, 1);

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

void TGraphWidget::saveGraph() {
    uint width = 800;
    uint height = 600;

    if(!TDialog::exportImageDimensionsDialog(this, width, height)) {
        return;
    }

    QStringList filters;
    filters << "SVG file (*.svg)"
            << "PNG file (*.png)"
            << "JPEG file (*.jpeg)"
            << "Any files (*)";

    QFileDialog saveDialog;
    saveDialog.setNameFilters(filters);
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setFileMode(QFileDialog::AnyFile);

    // TODO: open specific directory?
    // openDialog.setDirectory(m_projectDirectory);

    saveDialog.selectFile(m_graph->name());
    if (!saveDialog.exec()) return;

    QString selectedFilePath = saveDialog.selectedFiles().constFirst();

    if(selectedFilePath.endsWith(".svg")) {
        QSvgGenerator generator;
        generator.setFileName(selectedFilePath);
        generator.setSize(QSize(width, height));
        generator.setViewBox(QRect(0, 0, width, height));
        generator.setTitle(m_graph->name());

        renderGraph(&generator, width, height);
    } else {
        QPixmap pixmap(QSize(width, height));
        renderGraph(&pixmap, width, height);

        if(!pixmap.save(selectedFilePath)) {
            QMessageBox::warning(this, tr("Image save failed"), tr("Image save failed!"));
        }
    }
}

void TGraphWidget::renderGraph(QPaintDevice * paintDevice, uint width, uint height) {
    QPainter painter(paintDevice);
    double sx = double(width) / m_graph->width();
    double sy = double(height) / m_graph->height();
    painter.scale(sx, sy);
    m_graph->render(&painter);
    painter.end();
}
