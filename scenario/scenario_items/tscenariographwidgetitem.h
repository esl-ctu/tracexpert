// COPYRIGHT HEADER BEGIN
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
// COPYRIGHT HEADER END

#ifndef TSCENARIOCREATEGRAPHITEM_H
#define TSCENARIOCREATEGRAPHITEM_H

#include <QApplication>
#include <QString>
#include <QList>
#include <QDataStream>

#include "../tmainwindow.h"
#include "../tscenarioitem.h"

#include "../graphs/tcpagraph.h"

class TScenarioCreateGraphItem : public TScenarioItem {
    Q_OBJECT

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioGraphWidgetItem;
    }

    TScenarioCreateGraphItem() : TScenarioItem(tr("Create graph"), tr("This block creates a graph widget.")) {
        addFlowInputPort("flowIn");
        addDataInputPort("dataIn", "", tr("Data pased through this port will be passed to the graph widget."), "[byte array]");
        addFlowOutputPort("flowOut");

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        TConfigParam selectParam("Graph to display", "", TConfigParam::TType::TEnum, tr("Select the graph that should open."), false);
        selectParam.addEnumValue("");
        selectParam.addEnumValue("CPA");
        selectParam.addEnumValue("dummy");
        m_params.addSubParam(selectParam);

        TConfigParam configParam("Graph configuration", "", TConfigParam::TType::TDummy, tr("Specific configuration for the selected graph."));
        m_params.addSubParam(configParam);
    }

    TScenarioItem * copy() const override {
        return new TScenarioCreateGraphItem(*this);
    }

    /* TODO: find suitable icon
    const QString getIconResourcePath() const override {
        return ":/icons/log.png";
    }
    */

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Graph to display");
    }

    void updateParams(bool paramValuesChanged) override {
        TConfigParam * selectParam = m_params.getSubParamByName("Graph to display");

        if(paramValuesChanged) {
            m_params.removeSubParam("Graph configuration");

            TConfigParam configParam("Graph configuration", "", TConfigParam::TType::TDummy, tr("Specific configuration for the selected graph."));

            if(selectParam->getValue() == "CPA") {
                configParam = TCPAGraph().graphParams();
            }

            // TODO: add other graphs

            configParam.setName("Graph configuration");
            m_params.addSubParam(configParam);
        }
    }

    bool validateParamsStructure(TConfigParam params) override {
        bool iok = false;

        params.getSubParamByName("Graph to display", &iok);
        if(!iok) return false;

        params.getSubParamByName("Graph configuration", &iok);
        if(!iok) return false;

        return true;
    }

    TConfigParam setParams(TConfigParam params) override {
        if(!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the pre-init params."));
            return params;
        }

        bool shouldUpdate = shouldUpdateParams(params);
        m_params = params;
        updateParams(shouldUpdate);

        if(m_params.getState(true) == TConfigParam::TState::TError) {
            setState(TState::TError, tr("Block configuration contains errors!"));
        }
        else {
            resetState();
        }

        return m_params;
    }
    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        QString selectedGraphName = m_params.getSubParamByName("Graph to display")->getValue();
        TConfigParam configParam = *m_params.getSubParamByName("Graph configuration");
        QByteArray data = dataInputValues.value(getItemPortByName("dataIn"));

        TMainWindow * mainWindow = getMainWindow();
        QMetaObject::invokeMethod(
            mainWindow,
            [selectedGraphName, configParam, data, mainWindow]() {
                TGraph * graph = nullptr;

                if (selectedGraphName == "CPA") {
                    graph = new TCPAGraph();
                }

                // TODO: add other graphs

                if(graph) {
                    graph->setData(data);
                    graph->setGraphParams(configParam);
                    mainWindow->createGraphDockWidget(graph);
                }
            },
            Qt::QueuedConnection
        );

        return {};
    }

signals:
    void createGraphDockWidget(TGraph * graph);

private:
    TMainWindow * getMainWindow() {
        foreach (QWidget * widget, qApp->topLevelWidgets()) {
            if (TMainWindow * mainWindow = qobject_cast<TMainWindow *>(widget)) {
                return mainWindow;
            }
        }

        return nullptr;
    }
};

#endif // TSCENARIOCREATEGRAPHITEM_H
