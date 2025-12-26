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

#ifndef TSCENARIOBASICITEMS_H
#define TSCENARIOBASICITEMS_H

#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioFlowStartItem class represents a start point of a scenario.
 *
 * The class represents a start point of a scenario. It is a block with one output port.
 * There can be only one start point in a scenario.
 *
 */
class TScenarioFlowStartItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioFlowStartItem;
    }

    TScenarioFlowStartItem() : TScenarioItem(tr("Flow start"), tr("This block is the start point of the scenario.")) {
        setType(TItemAppearance::TFlowStart);
        addFlowOutputPort("flowOut");
    }

    TScenarioItem * copy() const override {
        return new TScenarioFlowStartItem(*this);
    }
};

/*!
 * \brief The TScenarioFlowEndItem class represents an end point of a scenario.
 *
 * The class represents an end point of a scenario. It is a block with one input port.
 * There can be multiple end points in a scenario.
 *
 */
class TScenarioFlowEndItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioFlowEndItem;
    }

    TScenarioFlowEndItem() : TScenarioItem(tr("Flow end"), tr("This block is the end point of the scenario.")) {
        setType(TItemAppearance::TFlowEnd);
        addFlowInputPort("flowIn");
    }

    TScenarioItem * copy() const override {
        return new TScenarioFlowEndItem(*this);
    }
};

/*!
 * \brief The TScenarioFlowMergeItem class represents a block that merges multiple flow paths.
 *
 * The class represents a block that merges multiple flow paths; e.g. after a condition.
 * It is a block with multiple input ports and one output port.
 * The number of input ports can be set in the configuration.
 *
 */
class TScenarioFlowMergeItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioFlowMergeItem;
    }

    TScenarioFlowMergeItem() : TScenarioItem(tr("Flow merge"), tr("This block merges multiple flow paths; e.g. after a condition.")) {
        setType(TItemAppearance::TFlowMerge);
        addFlowInputPort("flowIn1");
        addFlowInputPort("flowIn2");
        addFlowOutputPort("flowOut", "", tr("The flow from either of the inputs will continue through this port."));

        m_inputPortCount = 2;

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Input count", "2", TConfigParam::TType::TUInt, "Number of inputs into the block between 2 and 5.", false));
    }

    TScenarioItem * copy() const override {
        return new TScenarioFlowMergeItem(*this);
    }

    TConfigParam setParams(TConfigParam params) override {

        bool iok = false;
        params.getSubParamByName("Input count", &iok);

        if(!iok) {
            params.setState(TConfigParam::TState::TError, "Wrong structure of the pre-init params.");
            return params;
        }

        m_params = params;
        m_params.resetState(true);

        quint32 inputCount = m_params.getSubParamByName("Input count")->getValue().toUInt();
        if(inputCount < 2 || inputCount > 5) {
            m_params.getSubParamByName("Input count")->setState(TConfigParam::TState::TError, "Value has to be between 2 and 5.");
            return m_params;
        }

        if(inputCount != m_inputPortCount) {
            for(quint32 i = 2; i < std::max(inputCount, m_inputPortCount); i++) {
                if(i >= inputCount) {
                    // remove this port
                    removePort(QString("flowIn%1").arg(i+1));
                }
                else {
                    // create a new port
                    addFlowInputPort(QString("flowIn%1").arg(i+1));
                }
            }

            m_inputPortCount = inputCount;
        }

        return m_params;
    }

private:
    quint32 m_inputPortCount;

};

/*!
 * \brief The TScenarioConditionItem class represents a block that directs flow based on a set condition.
 *
 * The class represents a block that directs flow based on a set condition.
 * It is a block with one input data port for the boolean value and two output flow ports.
 *
 */
class TScenarioConditionItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioConditionItem;
    }

    TScenarioConditionItem() : TScenarioItem(tr("Condition"), tr("This block directs flow based on a set condition.")) {
        setType(TItemAppearance::TCondition);
        addFlowInputPort("flowIn");
        addDataInputPort("predicateIn", "", tr("Data passed through this port will be evaluated to be true/false."), "[any]");
        addFlowOutputPort("flowOutTrue", "", tr("If the input is evaluated to be TRUE, the flow will continue through this port."));
        addFlowOutputPort("flowOutFalse", "", tr("If the input is evaluated to be FALSE, the flow will continue through this port."));
    }

    TScenarioItem * copy() const override {
        return new TScenarioConditionItem(*this);
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        // evaluates bool value of first byte array byte
        QByteArray predicateIn = dataInputValues.value(this->getItemPortByName("predicateIn"));
        m_predicateInBoolValue = predicateIn.size() > 0 ? predicateIn.at(0) : false;

        return QHash<TScenarioItemPort *, QByteArray>();
    }
    
    TScenarioItemPort * getPreferredOutputFlowPort() override {
        return m_predicateInBoolValue ? this->getItemPortByName("flowOutTrue") : this->getItemPortByName("flowOutFalse");
    }

private:
    bool m_predicateInBoolValue;

};

#endif // TSCENARIOBASICITEMS_H
