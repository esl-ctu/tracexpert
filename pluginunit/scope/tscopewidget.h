#ifndef TOSCILLOSCOPEWIDGET_H
#define TOSCILLOSCOPEWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>
#include <QDialog>
#include <QStringList>
#include <QRadioButton>
#include <QSpinBox>
#include <QLabel>

#include "tscopemodel.h"
#include "widgets/tconfigparamwidget.h"
#include "../../eximport/texporthdfscopewizard.h"

class TDynamicRadioDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TDynamicRadioDialog(const QStringList &options,
                                 QWidget *parent = nullptr);

    QString selectedOption() const;

private:
    QList<QRadioButton*> m_radioButtons;
};

struct TScopeTraceData {
    size_t firstTraceIndex;
    size_t traces;
    size_t samples;
    TScope::TSampleType type;
    QList<QByteArray> buffers;
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

    void receiveTraces(size_t traces, size_t samples, TScope::TSampleType type, QList<QByteArray> buffers, bool overvoltage);

private slots:
    bool applyPostInitParam();

    void runOnceButtonClicked();
    void runButtonClicked();
    void stopButtonClicked();

    void clearTraceData(bool force = false);
    void saveTraceData();
    void exportHdf();

    void showPrevTrace();
    void showNextTrace();

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
        const T * buffer,
        size_t sampleOffset,
        size_t sampleCount);

    bool m_isDataIntendedForThisWidget;

    void setGUItoReady();
    void setGUItoRunning();

    QList<TScopeTraceData> m_traceDataList;
    void displayTrace(size_t traceIndex);

    size_t m_currentTraceNumber;
    size_t m_totalTraceCount;

    QPushButton * m_runOnceButton;
    QPushButton * m_runButton;
    QPushButton * m_stopButton;
    QPushButton * m_clearDataButton;
    QPushButton * m_saveDataButton;
    QPushButton * m_exportDataButton;

    QSpinBox * m_traceIndexSpinBox;
    QLabel * m_traceTotalLabel;
    void updateTraceIndexView();

    QPushButton * m_nextTraceButton;
    QPushButton * m_prevTraceButton;

    TConfigParamWidget * m_paramWidget;
    
    TExportHDFScopeWizard *m_exportWizard = nullptr;
    size_t m_samples;
    TScope::TSampleType m_type;
};

#endif // TOSCILLOSCOPEWIDGET_H
