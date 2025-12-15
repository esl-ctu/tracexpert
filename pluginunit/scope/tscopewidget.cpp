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

#include "tscopewidget.h"
#include "widgets/tconfigparamwidget.h"

TDynamicRadioDialog::TDynamicRadioDialog(const QStringList &options,
                                       QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Channel selection"));

    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("Please select the channel to save:")));

    // Create radio buttons dynamically
    bool first = true;
    for (const QString &opt : options)
    {
        QRadioButton *rb = new QRadioButton(opt, this);
        layout->addWidget(rb);

        if (first) {
            rb->setChecked(true);
            first = false;
        }

        m_radioButtons.append(rb);
    }

    // OK / Cancel buttons
    auto *btnBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(btnBox);
}

QString TDynamicRadioDialog::selectedOption() const
{
    for (QRadioButton *rb : m_radioButtons)
        if (rb->isChecked())
            return rb->text();

    return QString();
}

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
    // TODO: decide on a more suitable title text
    // m_axisY->setTitleText("Voltage");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_iconAxis = new QCategoryAxis();
    m_iconAxis->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    m_chart->addAxis(m_iconAxis, Qt::AlignRight);

    m_paramWidget = new TConfigParamWidget(m_scopeModel->postInitParams());
    m_paramWidget->setMinimumWidth(320);

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TScopeWidget::applyPostInitParam);

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

    QVBoxLayout * lowerLayout = new QVBoxLayout();
    lowerLayout->addWidget(lowerSplitter);
    lowerLayout->setContentsMargins(0, 0, 0, 0);

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

    m_clearDataButton = new QPushButton();
    m_clearDataButton->setIcon(QIcon(":/icons/delete.png"));
    m_clearDataButton->setIconSize(QSize(22, 22));
    m_clearDataButton->setToolTip("Clear trace data");
    m_clearDataButton->setEnabled(false);
    connect(m_clearDataButton, &QPushButton::clicked, this, &TScopeWidget::clearTraceData);

    m_saveDataButton = new QPushButton();
    m_saveDataButton->setIcon(QIcon(":/icons/save.png"));
    m_saveDataButton->setIconSize(QSize(22, 22));
    m_saveDataButton->setToolTip("Save trace data to file");
    m_saveDataButton->setEnabled(false);
    connect(m_saveDataButton, &QPushButton::clicked, this, &TScopeWidget::saveTraceData);

    m_prevTraceButton = new QPushButton();
    m_prevTraceButton->setIcon(QIcon(":/icons/prev.png"));
    m_prevTraceButton->setIconSize(QSize(22, 22));
    m_prevTraceButton->setToolTip("Previous trace");
    connect(m_prevTraceButton, &QPushButton::clicked, this, &TScopeWidget::showPrevTrace);

    m_currentTraceNumber = 0;
    m_totalTraceCount = 0;

    m_traceIndexSpinBox = new QSpinBox(this);
    m_traceIndexSpinBox->setRange(0, 0);
    m_traceIndexSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons); // hide up/down arrows
    m_traceIndexSpinBox->setFixedWidth(80);
    m_traceIndexSpinBox->setAlignment(Qt::AlignCenter);
    connect(m_traceIndexSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=](int newTrace) {
        if(newTrace < 1 || newTrace > m_totalTraceCount)
            return;
        m_currentTraceNumber = newTrace;
        updateTraceIndexView();
        displayTrace(m_currentTraceNumber-1);
    });

    m_traceTotalLabel = new QLabel(" of 0", this);
    m_traceTotalLabel->setFixedWidth(80);
    m_traceTotalLabel->setAlignment(Qt::AlignCenter);

    m_nextTraceButton = new QPushButton();
    m_nextTraceButton->setIcon(QIcon(":/icons/next.png"));
    m_nextTraceButton->setIconSize(QSize(22, 22));
    m_nextTraceButton->setToolTip("Next trace");
    connect(m_nextTraceButton, &QPushButton::clicked, this, &TScopeWidget::showNextTrace);

    QHBoxLayout * toolbarLayout = new QHBoxLayout;
    toolbarLayout->addWidget(m_runOnceButton);
    toolbarLayout->addWidget(m_runButton);
    toolbarLayout->addWidget(m_stopButton);
    toolbarLayout->addWidget(m_clearDataButton);
    toolbarLayout->addWidget(m_saveDataButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_prevTraceButton);
    toolbarLayout->addWidget(m_traceIndexSpinBox);
    toolbarLayout->addWidget(m_traceTotalLabel);
    toolbarLayout->addWidget(m_nextTraceButton);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addLayout(toolbarLayout);
    layout->addLayout(lowerLayout, 1);

    m_isDataIntendedForThisWidget = false;

    setLayout(layout);
}

bool TScopeWidget::applyPostInitParam() {

    if(m_totalTraceCount > 0){
        QMessageBox::StandardButton reply =
            QMessageBox::question(
                this,
                tr("Confirm deleting"),
                tr("Applying parameters will delete all measured data. Continue?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
                );

        if (reply != QMessageBox::Yes)
            return false;
    }

    TConfigParam param = m_scopeModel->setPostInitParams(m_paramWidget->param());
    m_paramWidget->setParam(param);

    if (param.getState(true) == TConfigParam::TState::TError) {
        qWarning("TScope parameters not set due to error state!");
        return false;
    }

    clearTraceData(true);

    return true;
}

void TScopeWidget::clearTraceData(bool force) {

    if(!force){
        QMessageBox::StandardButton reply =
            QMessageBox::question(
                this,
                tr("Confirm deleting"),
                tr("Are you sure you want to delete all measured traces?"),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No
                );

        if (reply != QMessageBox::Yes)
            return;
    }

    m_traceDataList.clear();

    m_totalTraceCount = 0;
    m_currentTraceNumber = 0;
    updateTraceIndexView();

    m_chart->removeAllSeries();

    m_clearDataButton->setEnabled(false);
    m_saveDataButton->setEnabled(false);
}

void TScopeWidget::saveTraceData() {

    QStringList options;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {
        if(channel.isEnabled())
            options << channel.getAlias();
    }

    TDynamicRadioDialog dlg(options, this);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString chosenChannel = dlg.selectedOption();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save traces"),
        QString(),                          // default path
        tr("Binary (*.dat);;All Files (*.*)")
        );

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        qCritical("Cannot open file for writing.");
        return;
    }


    for(size_t traceIndex = 0; traceIndex < m_totalTraceCount; traceIndex++){
        bool found = false;
        size_t traceDataListIndex = 0;
        while(traceDataListIndex < (size_t)m_traceDataList.size()) {
            if(m_traceDataList[traceDataListIndex].firstTraceIndex + m_traceDataList[traceDataListIndex].traces > traceIndex) {
                found = true;
                break;
            }

            traceDataListIndex++;
        }

        if(!found) {
            qWarning("Trying to save a trace with valid index, but data is missing!");
            return;
        }

        const TScopeTraceData traceData = m_traceDataList[traceDataListIndex];
        size_t traceBufferIndex = traceIndex - m_traceDataList[traceDataListIndex].firstTraceIndex;


        size_t channelIndex = 0;
        for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {

            if(!channel.isEnabled() || traceData.buffers[channelIndex] == nullptr || channel.getAlias() != chosenChannel) {
                channelIndex++;
                continue;
            }

            switch(traceData.type) {
            case TScope::TSampleType::TUInt8:
            case TScope::TSampleType::TInt8:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples)), traceData.samples * sizeof(uint8_t));
                break;

            case TScope::TSampleType::TUInt16:
            case TScope::TSampleType::TInt16:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples)), traceData.samples * sizeof(uint16_t));
                break;

            case TScope::TSampleType::TUInt32:
            case TScope::TSampleType::TInt32:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples)), traceData.samples * sizeof(uint32_t));
                break;

            case TScope::TSampleType::TReal32:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples)), traceData.samples * sizeof(float));
                break;

            case TScope::TSampleType::TReal64:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples)), traceData.samples * sizeof(double));
                break;
            }

            channelIndex++;
        }

    }

}

void TScopeWidget::setGUItoRunning() {
    m_runOnceButton->setEnabled(false);
    m_runButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_clearDataButton->setEnabled(false);
    m_saveDataButton->setEnabled(false);

    //clearTraceData();

    m_isDataIntendedForThisWidget = true;
}

void TScopeWidget::setGUItoReady() {
    m_runOnceButton->setEnabled(true);
    m_runButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_clearDataButton->setEnabled(m_totalTraceCount ? true : false);
    m_saveDataButton->setEnabled(m_totalTraceCount ? true : false);

    if(m_totalTraceCount){
        m_traceIndexSpinBox->setMinimum(1);
        m_traceIndexSpinBox->setMaximum((int)m_totalTraceCount);
    } else {
        m_traceIndexSpinBox->setMinimum(0);
        m_traceIndexSpinBox->setMaximum(0);
    }

    m_isDataIntendedForThisWidget = false;
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
    m_clearDataButton->setEnabled(false);
    m_saveDataButton->setEnabled(false);

    m_scopeModel->stop();
}

void TScopeWidget::showPrevTrace() {
    if(m_currentTraceNumber <= 1)
        return;

    m_currentTraceNumber--;

    updateTraceIndexView();
    displayTrace(m_currentTraceNumber-1);
}

void TScopeWidget::showNextTrace() {
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
    m_clearDataButton->setEnabled(false);
    m_saveDataButton->setEnabled(false);

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
    setGUItoReady();
}

void TScopeWidget::updateTraceIndexView() {
    m_prevTraceButton->setEnabled(m_currentTraceNumber > 1);
    m_nextTraceButton->setEnabled(m_currentTraceNumber < m_totalTraceCount);

    m_traceIndexSpinBox->setMinimum(1);
    m_traceIndexSpinBox->setMaximum((int)m_totalTraceCount);
    m_traceIndexSpinBox->setValue((int)m_currentTraceNumber);
    m_traceTotalLabel->setText(QString(" of %2").arg(m_totalTraceCount));
}

void TScopeWidget::receiveTraces(size_t traces, size_t samples, TScope::TSampleType type, QList<QByteArray> buffers, bool overvoltage) {
    // if the user did not start sampling from this widget, ignore any incoming data
    if(!m_isDataIntendedForThisWidget) {
        return;
    }

    // save data to internal buffers
    m_traceDataList.append(TScopeTraceData{m_totalTraceCount, traces, samples, type, buffers, overvoltage});

    // update GUI
    m_totalTraceCount += traces;
    m_currentTraceNumber = m_totalTraceCount;
    updateTraceIndexView();
    m_saveDataButton->setEnabled(true);

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
    while(traceDataListIndex < (size_t)m_traceDataList.size()) {
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

    const TScopeTraceData traceData = m_traceDataList[traceDataListIndex];
    size_t traceBufferIndex = traceIndex - m_traceDataList[traceDataListIndex].firstTraceIndex;

    m_chart->removeAllSeries();

    m_axisX->setRange(0, traceData.samples);
    updateAxes();

    QList<QLineSeries *> preparedLineSeries;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {
        if(!channel.isEnabled())
            continue;

        preparedLineSeries.append(new QLineSeries());
    }

    // this is used to control the order of appearance of the series,
    // as they are internally sorted by pointer inside QChart...
    std::sort(preparedLineSeries.begin(), preparedLineSeries.end());

    size_t channelIndex = 0;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {

        if(!channel.isEnabled() || traceData.buffers[channelIndex] == nullptr) {
            channelIndex++;
            continue;
        }

        QLineSeries * lineSeries = preparedLineSeries.first();
        preparedLineSeries.pop_front();

        switch(traceData.type) {
            case TScope::TSampleType::TUInt8:
                createLineSeries(lineSeries, channel, (uint8_t*) traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
            case TScope::TSampleType::TInt8:
                createLineSeries(lineSeries, channel, (int8_t*)  traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
            case TScope::TSampleType::TUInt16:
                createLineSeries(lineSeries, channel, (uint16_t*)traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
            case TScope::TSampleType::TInt16:
                createLineSeries(lineSeries, channel, (int16_t*) traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
            case TScope::TSampleType::TUInt32:
                createLineSeries(lineSeries, channel, (uint32_t*)traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
            case TScope::TSampleType::TInt32:
                createLineSeries(lineSeries, channel, (int32_t*) traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
            case TScope::TSampleType::TReal32:
                createLineSeries(lineSeries, channel, (float*)   traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
            case TScope::TSampleType::TReal64:
                createLineSeries(lineSeries, channel, (double*)  traceData.buffers[channelIndex].constData(), traceBufferIndex, traceData.samples); break;
        }

        channelIndex++;
    }

    // delete possible leftover prepared LineSeries
    qDeleteAll(preparedLineSeries);
}

void TScopeWidget::updateAxes() {
    qreal maxRange = 0;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {
        if(!channel.isEnabled())
            continue;
        qreal range = channel.getRange() + abs(channel.getOffset());
        if(range > maxRange)
            maxRange = range;
    }
    m_axisY->setRange(-maxRange, maxRange);
    updateIconAxis();
}


void TScopeWidget::updateIconAxis() {
    const QStringList oldLabels = m_iconAxis->categoriesLabels();
    for(const QString & label : oldLabels) {
        m_iconAxis->remove(label);
    }

    QMap<qreal, QString> newLabels;

    TScope::TTriggerStatus trigger = m_scopeModel->triggerStatus();
    if(trigger.getTriggerType() != TScope::TTriggerStatus::TTriggerType::TNone) {
        newLabels[trigger.getTriggerVoltage()] = "<span style=\"color: red; font-size: 12px;\">&#129032;</span>";
    }

    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {
        if(!channel.isEnabled())
            continue;

        if(channel.getOffset() != 0) {
            if(newLabels.contains(channel.getOffset())) {
                newLabels[channel.getOffset()] +=
                    QString("<span style=\"color: %1; font-size: 12px;\">%2</span>")
                        .arg(
                            channelColorCodes[channel.getIndex() % 8],
                            channel.getAlias()
                        );
            }
            else {
                newLabels[channel.getOffset()] =
                    QString("<span style=\"color: %1; font-size: 12px;\">%2</span>")
                        .arg(
                            channelColorCodes[channel.getIndex() % 8],
                            channel.getAlias()
                        );
            }
        }
    }

    if(!newLabels.isEmpty()) {
        m_iconAxis->setMin(m_axisY->min());
        m_iconAxis->setMax(m_axisY->max());

        // this has to be done in ascending order!
        for(auto [key, value] : newLabels.asKeyValueRange()) {
            m_iconAxis->append(value, key);
        }
    }
}

template <class T>
void TScopeWidget::createLineSeries(QLineSeries * lineSeries, TScope::TChannelStatus channel, const T * buffer, size_t traceBufferIndex, size_t sampleCount) {
    lineSeries->setUseOpenGL(true);
    lineSeries->setName(QString(tr("%1 [channel %2]")).arg(channel.getAlias()).arg(channel.getIndex()));
    lineSeries->setColor(QColor(channelColorCodes[channel.getIndex() % 8]));

    QVector<QPointF> pointList;
    qreal slope = (channel.getRange() * 2) / (channel.getMaxValue() - channel.getMinValue());

    qint32 idealPointCount = m_chart->plotArea().width();

    qreal value;
    if(sampleCount < idealPointCount) {
        // not too many samples, draw every sample as single point
        for (size_t i = 0; i < sampleCount; i++) {
            value = slope * (buffer[i + traceBufferIndex * sampleCount] - channel.getMinValue()) - channel.getRange();
            pointList.append(QPointF(i, value));
        }
    }
    else {
        // too many samples, use min/max downsampling
        size_t groupSampleSize = sampleCount / idealPointCount;

        qreal min, max;
        size_t sampleIndex = 0;
        while(sampleIndex < sampleCount) {
            qreal firstValue = slope * (buffer[sampleIndex + traceBufferIndex * sampleCount] - channel.getMinValue()) - channel.getRange();
            min = firstValue;
            max = firstValue;

            size_t loopEndIndex = (sampleIndex + groupSampleSize) > sampleCount ? sampleCount : sampleIndex + groupSampleSize;
            for (size_t i = sampleIndex + 1; i < loopEndIndex; i++) {
                qreal value = slope * (buffer[i + traceBufferIndex * sampleCount] - channel.getMinValue()) - channel.getRange();
                min = value < min ? value : min;
                max = value > max ? value : max;
            }

            pointList.append(QPointF(sampleIndex, min));
            pointList.append(QPointF(sampleIndex, max));

            sampleIndex += groupSampleSize;
        }
    }

    lineSeries->replace(pointList);

    m_chart->addSeries(lineSeries);

    lineSeries->attachAxis(m_axisX);
    lineSeries->attachAxis(m_axisY);
}
