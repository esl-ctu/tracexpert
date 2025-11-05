#ifndef TGRAPH_H
#define TGRAPH_H

#include <QChart>
#include "tconfigparam.h"

class TGraph : public QChart
{
    Q_OBJECT

public:
    TGraph(const QString & name, QGraphicsItem * parent = nullptr) : QChart(parent), m_name(name) { }

    QString name() {
        return m_name;
    }

    virtual TConfigParam setParams(TConfigParam params) {
        m_params = params;
        return m_params;
    }

    TConfigParam params() {
        return m_params;
    }

protected:
    QString m_name;
    TConfigParam m_params;

};

#endif // TGRAPH_H
