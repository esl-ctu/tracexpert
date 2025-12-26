// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Adam Švehla (initial author)

#ifndef TGRAPH_H
#define TGRAPH_H

#include <QWidget>
#include "tconfigparam.h"

class TGraph : public QWidget
{
    Q_OBJECT

public:
    TGraph(const QString & name, QWidget * parent = nullptr) : QWidget(parent), m_name(name) { }

    virtual TGraph * copy() const {
        TGraph * copy = new TGraph(m_name);
        copy->setGraphParams(m_graphParams);
        copy->setData(m_data);
        return copy;
    }

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
