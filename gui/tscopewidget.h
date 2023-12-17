#ifndef TOSCILLOSCOPEWIDGET_H
#define TOSCILLOSCOPEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>

#include "tscopemodel.h"
#include "tconfigparamwidget.h"

struct TScopeTraceData {
    size_t firstTraceIndex;
    size_t traces;
    size_t samples;
    TScope::TSampleType type;
    QList<quint8 *> buffers;
    bool overvoltage;
};


class TScopeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TScopeWidget(TScopeModel * scope, QWidget * parent = nullptr);

public slots:
    void updateChannelStatus();

    void runFailed();
    void stopFailed();
    void stopped();

    void downloadFailed();
    void tracesEmpty();

    void receiveTraces(size_t traces, size_t samples, TScope::TSampleType type, QList<quint8 *> buffers, bool overvoltage);

private slots:
    bool applyPostInitParam();

    void runOnceButtonClicked();
    void runButtonClicked();
    void stopButtonClicked();

    void prevTraceButtonClicked();
    void nextTraceButtonClicked();

private:
    TScopeModel * m_scopeModel;

    // QList<QLineSeries> m_lineSeriesList;
    QChart * m_chart;
    QValueAxis * m_axisX;
    QValueAxis * m_axisY;

    template <class T>
    void createLineSeries(TScope::TChannelStatus channel, T * buffer, size_t offset, size_t sampleCount, qreal typeMinValue, qreal typeMaxValue);

    void setGUItoReady();
    void setGUItoRunning();

    QList<TScopeTraceData> m_traceDataList;
    void displayTrace(size_t traceIndex);

    size_t m_currentTraceNumber;
    size_t m_totalTraceCount;

    QPushButton * m_runOnceButton;
    QPushButton * m_runButton;
    QPushButton * m_stopButton;

    QLineEdit * m_traceIndexLineEdit;
    void updateTraceIndexView();

    QPushButton * m_nextTraceButton;
    QPushButton * m_prevTraceButton;

    TConfigParamWidget * m_paramWidget;
};

#endif // TOSCILLOSCOPEWIDGET_H
