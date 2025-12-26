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

#ifndef TGRAPHWIDGET_H
#define TGRAPHWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

#include "tgraph.h"
#include "widgets/tconfigparamwidget.h"

class TGraphWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TGraphWidget(TGraph * graph, QWidget * parent = nullptr);

    void drawGraph();

private slots:
    bool applyParams();
    void interpretationChanged();

private:
    void saveGraph();
    void renderGraph(QPaintDevice * paintDevice, uint width, uint height);

    TGraph * m_graph;

    TConfigParamWidget * m_graphParamWidget;
    TConfigParamWidget * m_interpretationParamWidget;
};

#endif // TGRAPHWIDGET_H
