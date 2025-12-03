#ifndef TCPAGRAPH_H
#define TCPAGRAPH_H

#include <QValueAxis>
#include <QScatterSeries>
#include <QChart>
#include <QChartView>
#include <QLayout>
#include <QGraphicsLayout>

#include "tgraph.h"

#define ORD(n) (QString::number(n) + ((n%100>=11 && n%100<=13) ? "th" : QStringList({"th","st","nd","rd","th","th","th","th","th","th"}).at(n%10)))

class TCPAGraph : public TGraph
{
    Q_OBJECT

public:
    TCPAGraph() :
        TGraph("CPA"),
        m_hypothesisCount(256),
        m_sampleCount(1000),
        m_highlightBy(THighlightBy::TProbabilityRank),
        m_highlightByValue(1)
    {
        initGraphParams();
        initInterpretationParams();

        m_chart = new QChart();
        m_chart->layout()->setContentsMargins(0, 0, 0, 0);
        m_chart->setBackgroundRoundness(0);

        QChartView * chartView = new QChartView(m_chart);
        chartView->setRenderHint(QPainter::Antialiasing, false);
        chartView->setContentsMargins(0, 0, 0, 0);

        QLayout * layout = new QVBoxLayout();
        layout->addWidget(chartView);
        layout->setContentsMargins(0, 0, 0, 0);

        setLayout(layout);
    };

    TCPAGraph * copy() const override {
        return (TCPAGraph*)TGraph::copy();
    }

    TConfigParam setGraphParams(TConfigParam params) override {
        if(!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the params."));
            return params;
        }

        m_graphParams = params;

        // validate dimension params
        TConfigParam * hypothesisCountParam = m_graphParams.getSubParamByName("Number of key hypotheses");
        TConfigParam * sampleCountParam = m_graphParams.getSubParamByName("Number of samples");

        m_hypothesisCount = hypothesisCountParam->getValue().toInt();
        m_sampleCount = sampleCountParam->getValue().toInt();

        if(!m_data.isEmpty() && !validateMatrixDimensions()) {
            sampleCountParam->setState(TConfigParam::TState::TError, tr("Entered dimensions do not match data."));
            hypothesisCountParam->setState(TConfigParam::TState::TError, tr("Entered dimensions do not match data."));
        }
        else {
            sampleCountParam->resetState();
            hypothesisCountParam->resetState();
        }

        // validate highlighted series
        TConfigParam * highlightedSeriesParam = m_graphParams.getSubParamByName("Highlighted series");
        TConfigParam * rankOrValueParam = highlightedSeriesParam->getSubParamByName("Highlight by");
        TConfigParam * valueParam = highlightedSeriesParam->getSubParamByName("Value");

        m_highlightBy = rankOrValueParam->getValue() == "Probability rank" ? THighlightBy::TProbabilityRank : THighlightBy::TValue;
        m_highlightByValue = valueParam->getValue().toInt();

        if(!validateHighlightByValue()) {
            if(m_highlightBy == THighlightBy::TProbabilityRank) {
                valueParam->setState(TConfigParam::TState::TError, tr("Value must be in the {1; hypothesis count} range."));
            }
            else if(m_highlightBy == THighlightBy::TValue) {
                valueParam->setState(TConfigParam::TState::TError, tr("Value must be in the {0; hypothesis count-1} range."));
            }
        }
        else {
            valueParam->resetState();
        }

        return m_graphParams;
    }

    void drawGraph() override {
        initInterpretationParams();

        if(!validateMatrixDimensions()) {
            qWarning("CPA graph cannot be displayed, configured matrix dimensions do not match data.");
            return;
        }

        if(!validateHighlightByValue()) {
            qWarning("CPA graph cannot be displayed, highlighted series configuration invalid.");
            return;
        }

        resetAxes();        
        m_chart->removeAllSeries();

        QList<quint32> hypothesesByCorrelation;
        QList<double> absMaxCorrelationByHypothesis;
        createSortedHypothesisList(hypothesesByCorrelation, absMaxCorrelationByHypothesis);

        drawSeries(hypothesesByCorrelation);

        // set interpretation results
        createInterpretation(hypothesesByCorrelation, absMaxCorrelationByHypothesis);
    }

protected:
    enum class THighlightBy {
        TProbabilityRank = 0,
        TValue = 1
    };

    bool validateParamsStructure(TConfigParam params) {
        bool iok = false;

        params.getSubParamByName("Number of key hypotheses", &iok);
        if(!iok) return false;

        params.getSubParamByName("Number of samples", &iok);
        if(!iok) return false;

        TConfigParam * highlightedSeriesParam = params.getSubParamByName("Highlighted series", &iok);
        if(!iok) return false;

        highlightedSeriesParam->getSubParamByName("Highlight by", &iok);
        if(!iok) return false;

        highlightedSeriesParam->getSubParamByName("Value", &iok);
        if(!iok) return false;

        return true;
    }

    void initGraphParams() {
        m_graphParams = TConfigParam("Graph configuration", "", TConfigParam::TType::TDummy, "");
        m_graphParams.addSubParam(TConfigParam("Number of key hypotheses", QString("%1").arg(m_hypothesisCount), TConfigParam::TType::TUInt, tr("Number of key hypotheses.")));
        m_graphParams.addSubParam(TConfigParam("Number of samples", QString("%1").arg(m_sampleCount), TConfigParam::TType::TUInt, tr("Number of samples.")));

        TConfigParam highlightedSeriesParam("Highlighted series", "", TConfigParam::TType::TDummy, tr("Select which series to highlight."));

        TConfigParam rankOrValueParam("Highlight by", "Probability rank", TConfigParam::TType::TEnum, tr("Select if the value below represents probability rank or the key value."));
        rankOrValueParam.addEnumValue("Probability rank");
        rankOrValueParam.addEnumValue("Key value");
        highlightedSeriesParam.addSubParam(rankOrValueParam);

        TConfigParam valueParam("Value", "1", TConfigParam::TType::TUInt, tr("Probability rank/key value of series to highlight."));
        highlightedSeriesParam.addSubParam(valueParam);

        m_graphParams.addSubParam(highlightedSeriesParam);
    }

    void initInterpretationParams() {
        m_interpretationParams = TConfigParam("Interpretation", "", TConfigParam::TType::TDummy, tr("Interpretation of the displayed data."), true);
    }

    bool validateMatrixDimensions() {
        return (m_data.size() / sizeof(double)) == (m_hypothesisCount * m_sampleCount);
    }

    bool validateHighlightByValue() {
        if(m_highlightBy == THighlightBy::TProbabilityRank && (m_highlightByValue < 1 || m_highlightByValue > m_hypothesisCount)) {
            return false;
        }
        else if(m_highlightBy == THighlightBy::TValue && m_highlightByValue >= m_hypothesisCount) {
            return false;
        }

        return true;
    }

    void createSortedHypothesisList(QList<quint32> & hypothesesByCorrelation, QList<double> & absMaxCorrelationByHypothesis) {
        QList<QPair<quint32, double>> hypothesisCorrelationMap;
        for(quint32 hypothesis = 0; hypothesis < m_hypothesisCount; hypothesis ++) {
            double max = 0.0;
            for(quint32 sampleIdx = 0; sampleIdx < m_sampleCount; sampleIdx ++) {
                double value = ((double*)m_data.data())[hypothesis * m_sampleCount + sampleIdx];
                max = value > max ? value : max;
            }
            hypothesisCorrelationMap.append(qMakePair(hypothesis, max));
            absMaxCorrelationByHypothesis.append(max);
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

    void createInterpretation(const QList<quint32> & hypothesesByCorrelation, const QList<double> & absMaxCorrelationByHypothesis) {
        quint32 rank = 1;

        for (auto it = hypothesesByCorrelation.crbegin(); it != hypothesesByCorrelation.crend(); ++it) {
            quint32 hypothesis = *it;

            QString paramName = QString("%1 most probable hypothesis").arg(ORD(rank));
            QString value = QString("0x%1").arg(hypothesis, 0, 16);
            QString correlation = QString::number(absMaxCorrelationByHypothesis[hypothesis], 'f', 4);

            TConfigParam hypothesisParam(paramName, "", TConfigParam::TType::TDummy, tr(""), true);
            hypothesisParam.addSubParam(TConfigParam("Value", value, TConfigParam::TType::TString, tr(""), true));
            hypothesisParam.addSubParam(TConfigParam("Abs. max. correlation", correlation, TConfigParam::TType::TString, tr(""), true));
            m_interpretationParams.addSubParam(hypothesisParam);

            rank++;
        }

        emit interpretationChanged();
    }

    void createSeriesForHypothesis(quint32 hypothesis, QList<QScatterSeries *> & preparedSeries, double & min, double & max, bool highlight) {
        QScatterSeries * series =  preparedSeries.first();
        preparedSeries.pop_front();

        series->setUseOpenGL(true);
        series->setMarkerSize(5);
        series->setColor(highlight ? QColor(32, 159, 223) : QColorConstants::DarkGray);

        QVector<QPointF> pointList;
        for(quint32 sampleIdx = 0; sampleIdx < m_sampleCount; sampleIdx ++) {
            double value = ((double*)m_data.data())[hypothesis * m_sampleCount + sampleIdx];
            max = value > max ? value : max;
            min = value < min ? value : min;
            pointList.append(QPointF(sampleIdx, value));
        }
        series->replace(pointList);

        m_chart->addSeries(series);
        series->attachAxis(m_axisX);
        series->attachAxis(m_axisY);
    }

    void resetAxes() {
        // create X axis
        if(m_axisX) {
            m_chart->removeAxis(m_axisX);
        }
        m_axisX = new QValueAxis();
        m_axisX->setTitleText("Samples");
        m_axisX->setLabelFormat("%g");
        m_axisX->setRange(0, m_sampleCount);
        m_chart->addAxis(m_axisX, Qt::AlignBottom);

        // create Y axis
        if(m_axisY) {
            m_chart->removeAxis(m_axisY);
        }
        m_axisY = new QValueAxis();
        m_axisY->setTitleText("Pearson correlation coefficient");
        m_axisY->setLabelFormat("%g");
        m_axisY->setRange(-1, 1);
        m_chart->addAxis(m_axisY, Qt::AlignLeft);

        // hide chart legend
        m_chart->legend()->hide();
    }

    void drawSeries(QList<quint32> & hypothesesByCorrelation) {
        double max = -1.0;
        double min = 1.0;

        QScatterSeries * highlightedSeries = new QScatterSeries();
        QScatterSeries * backgroundSeries = new QScatterSeries();

        if(highlightedSeries < backgroundSeries) {
            std::swap(highlightedSeries, backgroundSeries);
        }

        highlightedSeries->setUseOpenGL(true);
        highlightedSeries->setMarkerSize(5);
        highlightedSeries->setColor(QColor(32, 159, 223));

        backgroundSeries->setUseOpenGL(true);
        backgroundSeries->setMarkerSize(5);
        backgroundSeries->setColor(QColorConstants::DarkGray);

        QVector<QPointF> highlightedSeriesPointList;
        QVector<QPointF> backgroundSeriesPointList;

        quint32 rank = m_hypothesisCount;
        for(quint32 hypothesis : hypothesesByCorrelation) {
            QVector<QPointF> * targetPointList = &backgroundSeriesPointList;

            if(Q_UNLIKELY(m_highlightBy == THighlightBy::TValue && hypothesis == m_highlightByValue) ||
                (m_highlightBy == THighlightBy::TProbabilityRank && rank == m_highlightByValue))
            {
                targetPointList = &highlightedSeriesPointList;
            }

            for(quint32 sampleIdx = 0; sampleIdx < m_sampleCount; sampleIdx ++) {
                double value = ((double*)m_data.data())[hypothesis * m_sampleCount + sampleIdx];
                max = value > max ? value : max;
                min = value < min ? value : min;
                targetPointList->append(QPointF(sampleIdx, value));
            }

            rank--;
        }

        highlightedSeries->replace(highlightedSeriesPointList);
        m_chart->addSeries(highlightedSeries);
        highlightedSeries->attachAxis(m_axisX);
        highlightedSeries->attachAxis(m_axisY);

        backgroundSeries->replace(backgroundSeriesPointList);
        m_chart->addSeries(backgroundSeries);
        backgroundSeries->attachAxis(m_axisX);
        backgroundSeries->attachAxis(m_axisY);

        m_axisY->setRange(min, max);
        m_axisY->applyNiceNumbers();
    }

    QChart * m_chart;

    quint32 m_hypothesisCount;
    quint32 m_sampleCount;

    THighlightBy m_highlightBy;
    quint32 m_highlightByValue;

    QValueAxis * m_axisX = nullptr;
    QValueAxis * m_axisY = nullptr;
};

#endif // TCPAGRAPH_H
