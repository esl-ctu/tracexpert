#include "tscopewidget.h"

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

#include "tconfigparamwidget.h"

TScopeWidget::TScopeWidget(TScopeModel * scope, QWidget * parent) : QWidget(parent), m_scopeModel(scope) {
    setWindowTitle(tr("Oscilloscope - %1").arg(m_scopeModel->name()));

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setMinimumSize(700, 300);

    connect(scope, &TScopeModel::channelsStatusChanged, this, &TScopeWidget::updateChannelStatus);

    connect(scope, &TScopeModel::runFailed, this, &TScopeWidget::runFailed);
    connect(scope, &TScopeModel::stopFailed, this, &TScopeWidget::stopFailed);
    connect(scope, &TScopeModel::downloadFailed, this, &TScopeWidget::downloadFailed);

    connect(scope, &TScopeModel::tracesDownloaded, this, &TScopeWidget::receiveTraces);
    connect(scope, &TScopeModel::tracesEmpty, this, &TScopeWidget::tracesEmpty);
    connect(scope, &TScopeModel::stopped, this, &TScopeWidget::stopped);

    m_chart = new QChart();
    QChartView * chartView = new QChartView(m_chart);

    m_axisX = new QValueAxis();
    m_axisX->setLabelFormat("%g");
    m_axisX->setTitleText("Samples");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Voltage");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_paramWidget = new TConfigParamWidget(m_scopeModel->postInitParams());

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TScopeWidget::applyPostInitParam);

    QVBoxLayout * paramLayout = new QVBoxLayout;
    paramLayout->addWidget(m_paramWidget);
    paramLayout->addWidget(applyButton);

    QHBoxLayout * lowerLayout = new QHBoxLayout;
    lowerLayout->addWidget(chartView, 1);
    lowerLayout->addLayout(paramLayout);

    QGroupBox * lowerGroupBox = new QGroupBox;
    lowerGroupBox->setLayout(lowerLayout);

    m_runOnceButton = new QPushButton();
    m_runOnceButton->setIcon(QIcon(":/icons/play.png"));
    m_runOnceButton->setIconSize(QSize(22, 22));
    m_runOnceButton->setToolTip("Sample once");
    connect(m_runOnceButton, &QPushButton::clicked, this, &TScopeWidget::runOnceButtonClicked);

    m_runButton = new QPushButton();
    m_runButton->setIcon(QIcon(":/icons/repeat.png"));
    m_runButton->setIconSize(QSize(22, 22));
    m_runButton->setToolTip("Sample repeatedly");
    connect(m_runButton, &QPushButton::clicked, this, &TScopeWidget::runButtonClicked);

    m_stopButton = new QPushButton();
    m_stopButton->setIcon(QIcon(":/icons/stop.png"));
    m_stopButton->setIconSize(QSize(22, 22));
    m_stopButton->setToolTip("Stop");
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked, this, &TScopeWidget::stopButtonClicked);

    m_prevTraceButton = new QPushButton();
    m_prevTraceButton->setIcon(QIcon(":/icons/prev.png"));
    m_prevTraceButton->setIconSize(QSize(22, 22));
    m_prevTraceButton->setToolTip("Previous trace");
    connect(m_prevTraceButton, &QPushButton::clicked, this, &TScopeWidget::prevTraceButtonClicked);

    m_currentTraceNumber = 0;
    m_totalTraceCount = 0;

    m_traceIndexLineEdit = new QLineEdit("0 of 0");
    m_traceIndexLineEdit->setEnabled(false);
    m_traceIndexLineEdit->setAlignment(Qt::AlignCenter);

    m_nextTraceButton = new QPushButton();
    m_nextTraceButton->setIcon(QIcon(":/icons/next.png"));
    m_nextTraceButton->setIconSize(QSize(22, 22));
    m_nextTraceButton->setToolTip("Next trace");
    connect(m_nextTraceButton, &QPushButton::clicked, this, &TScopeWidget::nextTraceButtonClicked);

    QHBoxLayout * toolbarLayout = new QHBoxLayout;
    toolbarLayout->addWidget(m_runOnceButton);
    toolbarLayout->addWidget(m_runButton);
    toolbarLayout->addWidget(m_stopButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_prevTraceButton);
    toolbarLayout->addWidget(m_traceIndexLineEdit);
    toolbarLayout->addWidget(m_nextTraceButton);

    QGroupBox * upperGroupBox = new QGroupBox;
    upperGroupBox->setLayout(toolbarLayout);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(upperGroupBox);
    layout->addWidget(lowerGroupBox);

    setLayout(layout);
}

bool TScopeWidget::applyPostInitParam() {
    TConfigParam param = m_scopeModel->setPostInitParams(m_paramWidget->param());
    if (param.getState(true) == TConfigParam::TState::TError) {
        qWarning("TScope parameters not set due to error state!");
        return false;
    }

    m_paramWidget->setParam(param);

    return true;
}

void TScopeWidget::setGUItoRunning() {
    m_runOnceButton->setEnabled(false);
    m_runButton->setEnabled(false);
    m_stopButton->setEnabled(true);

    m_traceDataList.clear();

    m_totalTraceCount = 0;
    m_currentTraceNumber = 0;
    updateTraceIndexView();
}

void TScopeWidget::setGUItoReady() {
    m_runOnceButton->setEnabled(true);
    m_runButton->setEnabled(true);
    m_stopButton->setEnabled(false);
}

void TScopeWidget::runOnceButtonClicked() {
    setGUItoRunning();
    m_scopeModel->runSingle();
}

void TScopeWidget::runButtonClicked() {
    setGUItoRunning();
    m_scopeModel->run();
}

void TScopeWidget::stopButtonClicked() {
    m_runOnceButton->setEnabled(false);
    m_runButton->setEnabled(false);
    m_stopButton->setEnabled(false);

    m_scopeModel->stop();
}

void TScopeWidget::prevTraceButtonClicked() {
    if(m_currentTraceNumber <= 1)
        return;

    m_currentTraceNumber--;

    updateTraceIndexView();
    displayTrace(m_currentTraceNumber-1);
}

void TScopeWidget::nextTraceButtonClicked() {
    if(m_currentTraceNumber >= m_totalTraceCount)
        return;

    m_currentTraceNumber++;

    updateTraceIndexView();
    displayTrace(m_currentTraceNumber-1);
}

void TScopeWidget::updateChannelStatus() {
    displayTrace(m_currentTraceNumber-1);
}

void TScopeWidget::runFailed() {    
    QMessageBox::critical(this, "Error", "Failed to start sampling");
    setGUItoReady();
}

void TScopeWidget::stopFailed() {
    m_runOnceButton->setEnabled(false);
    m_runButton->setEnabled(false);
    m_stopButton->setEnabled(true);

    QMessageBox::critical(this, "Error", "Failed to stop sampling");
}

void TScopeWidget::downloadFailed() {
    QMessageBox::critical(this, "Error", "Download failed");
    setGUItoReady();
}

void TScopeWidget::tracesEmpty() {
    QMessageBox::critical(this, "Error", "Traces empty");
    setGUItoReady();
}

void TScopeWidget::stopped() {
    qDebug("Download stopped!");
    setGUItoReady();
}

void TScopeWidget::updateTraceIndexView() {
    m_prevTraceButton->setEnabled(m_currentTraceNumber > 1);
    m_nextTraceButton->setEnabled(m_currentTraceNumber < m_totalTraceCount);

    m_traceIndexLineEdit->setText(QString("%1 of %2").arg(m_currentTraceNumber).arg(m_totalTraceCount));
}

void TScopeWidget::receiveTraces(size_t traces, size_t samples, TScope::TSampleType type, QList<quint8 *> buffers, bool overvoltage) {
    qDebug("Received traces!");

    // save data to internal buffers
    m_traceDataList.append({m_totalTraceCount, traces, samples, type, buffers, overvoltage});

    // update GUI
    m_totalTraceCount += traces;
    m_currentTraceNumber = m_totalTraceCount;
    updateTraceIndexView();

    // show latest trace on chart
    displayTrace(m_currentTraceNumber-1);
}

void TScopeWidget::displayTrace(size_t traceIndex) {
    if(traceIndex < 0 || traceIndex > m_totalTraceCount) {
        qWarning("Trying to display trace with invalid index!");
        return;
    }

    bool found = false;
    size_t traceDataListIndex = 0;
    while(traceDataListIndex < m_traceDataList.size()) {
        if(m_traceDataList[traceDataListIndex].firstTraceIndex + m_traceDataList[traceDataListIndex].traces > traceIndex) {
            found = true;
            break;
        }

        traceDataListIndex++;
    }

    if(!found) {
        qWarning("Trying to display trace with valid index, but data is missing!");
        return;
    }

    TScopeTraceData & traceData = m_traceDataList[traceDataListIndex];
    size_t sampleOffset = traceIndex - m_traceDataList[traceDataListIndex].firstTraceIndex;

    m_chart->removeAllSeries();

    qreal maxRange = 0;
    size_t maxSamples = 0;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {
        if(channel.getRange() > maxRange)
            maxRange = channel.getRange();

        if(traceData.samples > maxSamples)
            maxSamples = traceData.samples;
    }

    m_axisX->setRange(0, maxSamples);
    m_axisY->setRange(-maxRange*1.2f, maxRange*1.2f);


    size_t channelIndex = 0;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {

        if(!channel.isEnabled() || traceData.buffers[channelIndex] == nullptr) {
            channelIndex++;
            continue;
        }

        switch(traceData.type) {
            case TScope::TSampleType::TUInt8:
                createLineSeries(channel, (uint8_t*) traceData.buffers[channelIndex], sampleOffset, traceData.samples, 0, UINT8_MAX); break;
            case TScope::TSampleType::TInt8:
                createLineSeries(channel, (int8_t*)  traceData.buffers[channelIndex], sampleOffset, traceData.samples, INT8_MIN, INT8_MAX); break;
            case TScope::TSampleType::TUInt16:
                createLineSeries(channel, (uint16_t*)traceData.buffers[channelIndex], sampleOffset, traceData.samples, 0, UINT16_MAX); break;
            case TScope::TSampleType::TInt16:
                createLineSeries(channel, (int16_t*) traceData.buffers[channelIndex], sampleOffset, traceData.samples, INT16_MIN, INT16_MAX); break;
            case TScope::TSampleType::TUInt32:
                createLineSeries(channel, (uint32_t*)traceData.buffers[channelIndex], sampleOffset, traceData.samples, 0, UINT32_MAX); break;
            case TScope::TSampleType::TInt32:
                createLineSeries(channel, (int32_t*) traceData.buffers[channelIndex], sampleOffset, traceData.samples, INT32_MIN, INT32_MAX); break;
            case TScope::TSampleType::TReal32:
                createLineSeries(channel, (float*)   traceData.buffers[channelIndex], sampleOffset, traceData.samples, 0, 0); break;
            case TScope::TSampleType::TReal64:
                createLineSeries(channel, (double*)  traceData.buffers[channelIndex], sampleOffset, traceData.samples, 0, 0); break;
        }

        channelIndex++;
    }
}

template <class T>
void TScopeWidget::createLineSeries(TScope::TChannelStatus channel, T * buffer, size_t sampleOffset, size_t sampleCount, qreal typeMinValue, qreal typeMaxValue) {

    QLineSeries * lineSeries = new QLineSeries();
    lineSeries->setName(QString(tr("%1 [channel %2]")).arg(channel.getAlias()).arg(channel.getIndex()));
    lineSeries->setColor(channelColors[channel.getIndex()]);

    m_chart->addSeries(lineSeries);

    lineSeries->attachAxis(m_axisX);
    lineSeries->attachAxis(m_axisY);


    QList<QPointF> pointList;

    if(typeMaxValue == 0) { // TReal32, TReal64
        for (size_t i = sampleOffset; i < sampleCount; i++) {
            pointList.append(QPointF(i, buffer[i]));
        }
    }
    else if(typeMinValue == 0) { // unsigned types
        for (size_t i = sampleOffset; i < sampleCount; i++) {
            qreal lambda = buffer[i] / typeMaxValue;
            qreal value = lambda * (2 * channel.getRange()) - channel.getRange();
            pointList.append(QPointF(i, value));
        }
    }
    else { // signed types
        for (size_t i = sampleOffset; i < sampleCount; i++) {
            qreal lambda = buffer[i] > 0 ? (buffer[i] / typeMaxValue) : -(buffer[i] / typeMinValue);
            qreal value = lambda * channel.getRange();
            pointList.append(QPointF(i, value));
        }
    }

    lineSeries->replace(pointList);
}
