#ifndef TOSCILLOSCOPEWIDGET_H
#define TOSCILLOSCOPEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

#include "tscopemodel.h"
#include "tconfigparamwidget.h"

struct TScopeTraceData {
    ~TScopeTraceData() {
        // correct way to delete using the same operator ([])
        for(quint8 * buffer : buffers) {
            delete [] buffer;
        }
    }

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
    ~TScopeWidget();

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
    const QList<const char *> channelColorCodes = { "#711415", "#f8cc1b", "#ae311e", "#b5be2f", "#f37324", "#72b043", "#f6a020", "#007f4e" };

    TScopeModel * m_scopeModel;

    QChart * m_chart;
    QValueAxis * m_axisX;
    QValueAxis * m_axisY;
    QCategoryAxis * m_iconAxis;

    void updateAxes();
    void updateIconAxis();    

    template <class T>
    void createLineSeries(
        QLineSeries * lineSeries,
        TScope::TChannelStatus channel,
        T * buffer,
        size_t sampleOffset,
        size_t sampleCount);

    void setGUItoReady();
    void setGUItoRunning();

    QList<TScopeTraceData *> m_traceDataList;
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
