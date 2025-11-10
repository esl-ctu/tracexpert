#ifndef TCPAGRAPH_H
#define TCPAGRAPH_H

#include <QValueAxis>
#include <QScatterSeries>

#include "tgraph.h"

class TCPAGraph : public TGraph
{
    Q_OBJECT

public:
    TCPAGraph() : TGraph("CPA"), m_hypothesisCount(256), m_sampleCount(1000) {
        initParams();
    };

    TCPAGraph(const QByteArray & matrix) : TGraph("CPA"), m_hypothesisCount(256), m_sampleCount(1000), m_matrix(matrix) {
        initParams();
        drawGraph();
    }

    TConfigParam setParams(TConfigParam params) override {
        if(!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the params."));
            return params;
        }

        m_params = params;

        TConfigParam * hypothesisCountParam = m_params.getSubParamByName("Number of key hypotheses");
        TConfigParam * sampleCountParam = m_params.getSubParamByName("Number of samples");

        m_hypothesisCount = hypothesisCountParam->getValue().toInt();
        m_sampleCount = sampleCountParam->getValue().toInt();

        if(!m_matrix.isEmpty() && !validateMatrixDimensions()) {
            sampleCountParam->setState(TConfigParam::TState::TError, tr("Entered dimensions do not match data."));
            hypothesisCountParam->setState(TConfigParam::TState::TError, tr("Entered dimensions do not match data."));
        }
        else {
            sampleCountParam->resetState();
            hypothesisCountParam->resetState();
        }

        drawGraph();
        return m_params;
    }

private:
    bool validateParamsStructure(TConfigParam params) {
        bool iok = false;

        params.getSubParamByName("Number of key hypotheses", &iok);
        if(!iok) return false;

        params.getSubParamByName("Number of samples", &iok);
        if(!iok) return false;

        return true;
    }

    void initParams() {
        m_params = TConfigParam("Graph configuration and interpretation", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Number of key hypotheses", QString("%1").arg(m_hypothesisCount), TConfigParam::TType::TUInt, tr("Number of key hypotheses.")));
        m_params.addSubParam(TConfigParam("Number of samples", QString("%1").arg(m_sampleCount), TConfigParam::TType::TUInt, tr("Number of samples.")));
    }

    void clearGraph() {
        legend()->hide();
        removeAllSeries();
    }

    bool validateMatrixDimensions() {
        return (m_matrix.size() / sizeof(double)) == (m_hypothesisCount * m_sampleCount);
    }

    void createSortedHypothesisList(QList<quint32> & hypothesesByCorrelation) {
        QList<QPair<quint32, double>> hypothesisCorrelationMap;
        for(quint32 hypothesis = 0; hypothesis < m_hypothesisCount; hypothesis ++) {
            double max = 0.0;
            for(quint32 sampleIdx = 0; sampleIdx < m_sampleCount; sampleIdx ++) {
                double value = ((double*)m_matrix.data())[hypothesis * m_sampleCount + sampleIdx];
                max = value > max ? value : max;
            }
            hypothesisCorrelationMap.append(qMakePair(hypothesis, max));
        }

        std::sort(
            hypothesisCorrelationMap.begin(),
            hypothesisCorrelationMap.end(),
            [](const QPair<quint32, double> &a, const QPair<quint32, double> &b) {
                return a.second < b.second;
            }
        );

        for (QPair<quint32, double> & pair : hypothesisCorrelationMap) {
            hypothesesByCorrelation.append(pair.first);
        }
    }

    void prepareSeries(QList<QScatterSeries *> & series, quint32 count) {
        for(quint32 i = 0; i < count; i++) {
            series.append(new QScatterSeries());
        }

        // this is used to control the order of appearance of the series,
        // as they are internally sorted by pointer inside QChart...
        std::sort(series.begin(), series.end());
    }

    void drawGraph() {
        // clear old series
        clearGraph();

        // clear old interpretation results
        m_params.removeSubParam("Interpretation");

        // validate matrix dimensions
        if(!validateMatrixDimensions()) {
            qWarning("CPA graph cannot be displayed, configured matrix dimensions do not match data.");
            return;
        }

        // create X axis
        if(!m_axisX) {
            m_axisX = new QValueAxis();
            m_axisX->setTitleText("Samples");
            m_axisX->setLabelFormat("%g");
            m_axisX->setRange(0, m_sampleCount);
            addAxis(m_axisX, Qt::AlignBottom);
        }

        // create Y axis
        if(!m_axisY) {
            m_axisY = new QValueAxis();
            m_axisY->setTitleText("Pearson correlation coefficient");
            m_axisY->setLabelFormat("%g");
            addAxis(m_axisY, Qt::AlignLeft);
        }

        QList<quint32> hypothesesByCorrelation;
        createSortedHypothesisList(hypothesesByCorrelation);

        QList<QScatterSeries *> preparedSeries;
        prepareSeries(preparedSeries, m_hypothesisCount);

        double max = -1.0;
        double min = 1.0;

        // draw series
        for(quint32 hypothesis : hypothesesByCorrelation) {
            QScatterSeries * series = preparedSeries.first();
            preparedSeries.pop_front();

            series->setUseOpenGL(true);
            series->setMarkerSize(5);

            if(preparedSeries.size() > 0) {
                series->setColor(QColorConstants::DarkGray);
            }

            QVector<QPointF> pointList;
            for(quint32 sampleIdx = 0; sampleIdx < m_sampleCount; sampleIdx ++) {
                double value = ((double*)m_matrix.data())[hypothesis * m_sampleCount + sampleIdx];
                max = value > max ? value : max;
                min = value < min ? value : min;
                pointList.append(QPointF(sampleIdx, value));
            }
            series->replace(pointList);

            addSeries(series);
            series->attachAxis(m_axisX);
            series->attachAxis(m_axisY);
        }

        m_axisY->setRange(min, max);
        m_axisY->applyNiceNumbers();

        // set interpretation results
        TConfigParam interpretParam(TConfigParam("Interpretation", "", TConfigParam::TType::TDummy, tr("Interpretation of the displayed data."), true));
        TConfigParam bestHypothesisValueParam("Most probable key hypothesis value", "", TConfigParam::TType::TString, tr(""), true);
        bestHypothesisValueParam.setValue(QString("0x%1").arg(hypothesesByCorrelation[m_hypothesisCount-1], 0, 16));
        interpretParam.addSubParam(bestHypothesisValueParam);
        m_params.addSubParam(interpretParam);
    }

    quint32 m_hypothesisCount;
    quint32 m_sampleCount;

    QByteArray m_matrix;

    QValueAxis * m_axisX = nullptr;
    QValueAxis * m_axisY = nullptr;
};

#endif // TCPAGRAPH_H
