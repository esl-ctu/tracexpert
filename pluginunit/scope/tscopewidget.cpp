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
#include <QSharedPointer>
#include <QJsonObject>
#include <QJsonDocument>

#include "tscopewidget.h"
#include "widgets/tconfigparamwidget.h"
#include "../../eximport/texporthdfscopewizard.h"
#include "../../eximport/thdfsession.h"

TDynamicRadioDialog::TDynamicRadioDialog(const QStringList &options,
                                       QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Channel selection"));

    auto *layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel(tr("Please select the channel to export:")));

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
    m_saveDataButton->setIcon(QIcon(":/icons/exportraw.png"));
    m_saveDataButton->setIconSize(QSize(22, 22));
    m_saveDataButton->setToolTip("Export raw data to file");
    m_saveDataButton->setEnabled(false);
    connect(m_saveDataButton, &QPushButton::clicked, this, &TScopeWidget::saveTraceData);

    m_exportDataButton = new QPushButton();
    m_exportDataButton->setIcon(QIcon(":/icons/exporthdf.png"));
    m_exportDataButton->setIconSize(QSize(22, 22));
    m_exportDataButton->setToolTip("Export data to HDF5");
    m_exportDataButton->setEnabled(false);
    connect(m_exportDataButton, &QPushButton::clicked, this, &TScopeWidget::exportHdf);

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
    toolbarLayout->addWidget(m_exportDataButton);
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
    m_exportDataButton->setEnabled(false);
}

static QString triggerTypeToString(TScope::TTriggerStatus::TTriggerType t)
{
    switch (t) {
    case TScope::TTriggerStatus::TTriggerType::TNone:
        return QStringLiteral("none");
    case TScope::TTriggerStatus::TTriggerType::TRising:
        return QStringLiteral("rising");
    case TScope::TTriggerStatus::TTriggerType::TFalling:
        return QStringLiteral("falling");
    case TScope::TTriggerStatus::TTriggerType::TRisingOrFalling:
        return QStringLiteral("rising_or_falling");
    case TScope::TTriggerStatus::TTriggerType::TAbove:
        return QStringLiteral("above");
    case TScope::TTriggerStatus::TTriggerType::TBelow:
        return QStringLiteral("below");
    case TScope::TTriggerStatus::TTriggerType::TOther:
        return QStringLiteral("other");
    }

    return QStringLiteral("unknown");
}

class TWizLog
{
public:
    void critical(const QString &msg)
    {
        add("Fail: ", msg);
        qCritical() << msg;
    }

    void success(const QString &msg)
    {
        add("Success: ", msg);
    }

    QString text()
    {
        flushPending();
        m_lastMessage.clear();
        m_lastCount = 0;
        return m_lines.join('\n');
    }

private:
    void add(const QString &prefix, const QString &msg)
    {
        if (m_lastMessage == msg) {
            ++m_lastCount;
            return;
        }

        flushPending();

        m_lastMessage = msg;
        m_lastCount = 1;

        m_lines << (prefix + msg);
    }

    void flushPending()
    {
        if (m_lastMessage.isEmpty())
            return;

        if (m_lastCount > 1) {
            m_lines << QString("(%1x) %2")
            .arg(m_lastCount)
                .arg(m_lastMessage);
        }
    }

private:
    QStringList m_lines;
    QString m_lastMessage;
    int m_lastCount = 0;
};

void TScopeWidget::exportHdf() {

    TWizLog log;

    // List enabled channels
    QStringList channels;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {
        if(channel.isEnabled())
            channels << channel.getAlias();
    }

    if (m_exportWizard) {
        m_exportWizard->close();
        m_exportWizard->deleteLater(); // safe Qt deletion
        m_exportWizard = nullptr;
    }

    m_exportWizard = new TExportHDFScopeWizard(this);

    // config the wizard
    m_exportWizard->setInitialFileName("");
    m_exportWizard->setTraceInfo(m_totalTraceCount);
    m_exportWizard->setChannels(channels);
    m_exportWizard->setTraceFormat(m_samples, m_type);

    int r = m_exportWizard->exec();

    if (r == TExportHDFScopeWizard::Result_Paused) {

        // read input data from the export wizard

        const QString outFile = m_exportWizard->outputFile();
        const auto targets = m_exportWizard->channelTargets();
        QSharedPointer<THdfSession> session = m_exportWizard->hdfSession();
        bool exportAll = m_exportWizard->field("exportAllTraces").toBool();

        // write data to HDF file
        if (!session || !session->isOpen()) {
            log.critical("No open HDF5 session; cannot export.");
            m_exportWizard->showResultsPage(log.text());
            m_exportWizard->exec();
            m_exportWizard->deleteLater();
            m_exportWizard = nullptr;
            return;
        }

        for (const TExportHDFScopeWizard::TChannelTarget &t : targets) {
            const QString alias        = t.alias;
            const QString tracesPath   = THdfSession::normalizePath(t.tracesDataset);
            const QString metadataPath = THdfSession::normalizePath(t.metadataGroup);

            qDebug() << "[ExportCaller] Channel:" << alias
                     << "traces:" << tracesPath
                     << "meta:" << metadataPath
                     << "all:" << exportAll;


            THdfSession::TraceAppendHandle h;

            // --- Append traces

            size_t startTraceIdx = 0;
            size_t totalTraceCount = m_totalTraceCount;
            if(!exportAll){
                startTraceIdx = m_currentTraceNumber - 1;
                totalTraceCount = 1;
            }

            if (!session->beginAppendTraces(tracesPath, m_samples, m_type, totalTraceCount, h)){
                log.critical("Failed to open the HDF dataset: " + tracesPath);
                continue;
            }

            size_t first_trace = h.nextRow;
            TScope::TChannelStatus channelStatus(0, 0, 0, 0, 0, 0, 0); // will be set later

            bool ok = true;

            for(size_t traceIndex = startTraceIdx; traceIndex < startTraceIdx + totalTraceCount; traceIndex++){
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
                    log.critical("Trying to save a trace with valid index, but data is missing!");
                    ok = false;
                    continue;
                }

                const TScopeTraceData traceData = m_traceDataList[traceDataListIndex];
                size_t traceBufferIndex = traceIndex - m_traceDataList[traceDataListIndex].firstTraceIndex;


                size_t channelIndex = 0;
                bool foundChIdx = false;


                for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {

                    if(!channel.isEnabled() || traceData.buffers[channelIndex] == nullptr || channel.getAlias() != alias) {
                        channelIndex++;
                        continue;
                    } else {
                        foundChIdx = true;
                        channelStatus = channel;
                        break;
                    }

                }

                if(!foundChIdx) {
                    log.critical("Exported channel was not found in the oscilloscope widget!");
                    ok = false;
                    continue;
                }

                switch(traceData.type) {
                case TScope::TSampleType::TUInt8:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(uint8_t))), traceData.type, traceData.samples * sizeof(uint8_t))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                case TScope::TSampleType::TInt8:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(int8_t))), traceData.type, traceData.samples * sizeof(int8_t))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                case TScope::TSampleType::TUInt16:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(uint16_t))), traceData.type, traceData.samples * sizeof(uint16_t))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                case TScope::TSampleType::TInt16:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(int16_t))), traceData.type, traceData.samples * sizeof(int16_t))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                case TScope::TSampleType::TUInt32:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(uint32_t))), traceData.type, traceData.samples * sizeof(uint32_t))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                case TScope::TSampleType::TInt32:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(int32_t))), traceData.type, traceData.samples * sizeof(int32_t))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                case TScope::TSampleType::TReal32:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(float))), traceData.type, traceData.samples * sizeof(float))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                case TScope::TSampleType::TReal64:
                    if (!session->appendTraceRow(h, reinterpret_cast<const void *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(double))), traceData.type, traceData.samples * sizeof(double))){
                        log.critical("Failed to append a trace into HDF file.");
                        ok = false;
                    }
                    break;
                }

            }

            session->endAppendTraces(h);

            if(ok) log.success("Successfully exported " + QString::number(totalTraceCount) + " traces to " + tracesPath);
            ok = true;

            // --- Append metadata

            // ssize_t first_trace
            size_t trace_count = totalTraceCount;
            QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

            // TScope::TChannelStatus channelStatus
            TScope::TTriggerStatus triggerStatus = m_scopeModel->triggerStatus();
            TScope::TTimingStatus timingStatus = m_scopeModel->timingStatus();

            QJsonObject JChannel;
            JChannel["alias"] = channelStatus.getAlias();
            JChannel["index"] = channelStatus.getIndex();
            JChannel["maxValue"] = channelStatus.getMaxValue();
            JChannel["minValue"] = channelStatus.getMinValue();
            JChannel["offset"] = channelStatus.getOffset();
            JChannel["range"] = channelStatus.getRange();
            QJsonObject JTrigger;
            JTrigger["sourceIndex"] = triggerStatus.getTriggerSourceIndex();
            JTrigger["type"] = triggerTypeToString(triggerStatus.getTriggerType());
            JTrigger["voltage"] = triggerStatus.getTriggerVoltage();
            QJsonObject JTiming;
            JTiming["preTriggerSamples"] = static_cast<qint64>(timingStatus.getPreTriggerSamples());
            JTiming["postTriggerSamples"] = static_cast<qint64>(timingStatus.getPostTriggerSamples());
            JTiming["capturesPerRun"] = static_cast<qint64>(timingStatus.getCapturesPerRun());
            JTiming["samplePeriod"] = timingStatus.getSamplePeriod();
            QJsonObject JRoot;
            JRoot["channel"] = JChannel;
            JRoot["trigger"] = JTrigger;
            JRoot["timing"] = JTiming;
            QJsonDocument JDoc(JRoot);
            const QString settings = QString::fromUtf8(JDoc.toJson(QJsonDocument::Compact));


            if (!session->appendTracesMetadataRecord(metadataPath, first_trace, trace_count, timestamp, settings)){
                log.critical(QString("Failed to append metadata to the HDF group: %1").arg(metadataPath));
                ok = false;
            }

            if(ok) log.success("Successfully exported metadata to " + metadataPath);

        }

        // give feedback to the wizard and launch it again
        m_exportWizard->showResultsPage(log.text());
        m_exportWizard->exec();

    } else {
        QMessageBox::warning(this, tr("Error"), tr("The export was cancelled."));
    }

    m_exportWizard->deleteLater();
    m_exportWizard = nullptr;
    return;

}

void TScopeWidget::saveTraceData() {

    QStringList channels;
    for(TScope::TChannelStatus channel : m_scopeModel->channelsStatus()) {
        if(channel.isEnabled())
            channels << channel.getAlias();
    }

    TDynamicRadioDialog dlg(channels, this);

    if (dlg.exec() != QDialog::Accepted)
        return;

    QString chosenChannel = dlg.selectedOption();

    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("Save traces"),
        QString(),                          // default path
        tr("Binary (*.dat);;All Files (*.*)")
        );

    // Strip trailing dots
    while (fileName.endsWith('.'))
        fileName.chop(1);

    if (fileName.isEmpty())
        return;

    QFileInfo fi(fileName);
    if (fi.suffix().isEmpty()) {
        fileName += ".dat";
    }

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
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(uint8_t))), traceData.samples * sizeof(uint8_t));
                break;

            case TScope::TSampleType::TUInt16:
            case TScope::TSampleType::TInt16:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(uint16_t))), traceData.samples * sizeof(uint16_t));
                break;

            case TScope::TSampleType::TUInt32:
            case TScope::TSampleType::TInt32:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(uint32_t))), traceData.samples * sizeof(uint32_t));
                break;

            case TScope::TSampleType::TReal32:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(float))), traceData.samples * sizeof(float));
                break;

            case TScope::TSampleType::TReal64:
                file.write(reinterpret_cast<const char *>((traceData.buffers[channelIndex].constData()) + (traceBufferIndex * traceData.samples * sizeof(double))), traceData.samples * sizeof(double));
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
    m_exportDataButton->setEnabled(false);

    //clearTraceData();

    m_isDataIntendedForThisWidget = true;
}

void TScopeWidget::setGUItoReady() {
    m_runOnceButton->setEnabled(true);
    m_runButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_clearDataButton->setEnabled(m_totalTraceCount ? true : false);
    m_saveDataButton->setEnabled(m_totalTraceCount ? true : false);
    m_exportDataButton->setEnabled(m_totalTraceCount ? true : false);

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
    m_exportDataButton->setEnabled(false);

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
    m_exportDataButton->setEnabled(false);

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
    m_samples = samples;
    m_type = type;
    
    // update GUI
    m_totalTraceCount += traces;
    m_currentTraceNumber = m_totalTraceCount;
    updateTraceIndexView();
    m_saveDataButton->setEnabled(true);
    m_exportDataButton->setEnabled(true);

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
