#ifndef TGRAPH_H
#define TGRAPH_H

#include <QWidget>
#include "tconfigparam.h"

class TGraph : public QWidget
{
    Q_OBJECT

public:
    TGraph(const QString & name, QWidget * parent = nullptr) : QWidget(parent), m_name(name) { }

    QString name() {
        return m_name;
    }

    virtual void drawGraph() { }

    virtual TConfigParam setGraphParams(TConfigParam params) {
        m_graphParams = params;
        return m_graphParams;
    }

    TConfigParam graphParams() {
        return m_graphParams;
    }

    TConfigParam interpretationParams() {
        return m_interpretationParams;
    }

    void setData(QByteArray data) {
        m_data = data;
    }

signals:
    void interpretationChanged();

protected:
    QString m_name;
    TConfigParam m_graphParams;
    TConfigParam m_interpretationParams;

    QByteArray m_data;
};

#endif // TGRAPH_H
