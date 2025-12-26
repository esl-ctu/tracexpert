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

#ifndef TSCENARIOVARIABLEWRITEITEM_H
#define TSCENARIOVARIABLEWRITEITEM_H

#include <QIODevice>
#include "../tscenarioitem.h"

/*!
 * \brief ...
 *
 */
class TScenarioVariableWriteItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioVariableWriteItem;
    }

    TScenarioVariableWriteItem() : TScenarioItem(tr("Variable: write"), tr("This block allows writing to a variable.")) {
        addDataInputPort("dataIn", "", "", "[any]");

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Block name", "Variable: write", TConfigParam::TType::TString, tr("Display name of the block."), false));
        m_params.addSubParam(TConfigParam("Variable name", "", TConfigParam::TType::TString, tr("Variable name."), false));

        m_subtitle = "missing variable name";
        setState(TState::TError, tr("Missing variable name."));
    }

    TScenarioItem * copy() const override {
        return new TScenarioVariableWriteItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/variable.png";
    }

    QString variableName() {
        return m_params.getSubParamByName("Variable name")->getValue();
    }

    bool prepare() override {
        if(getState() == TState::TError) {
            return false;
        }

        return true;
    }

    bool validateParamsStructure(TConfigParam params) {
        bool iok = false;

        params.getSubParamByName("Block name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Variable name", &iok);
        if(!iok) return false;

        return true;
    }

    TConfigParam setParams(TConfigParam params) override {
        if(!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the pre-init params."));
            return params;
        }

        m_params = params;
        m_params.resetState(true);

        if(m_title != params.getSubParamByName("Block name")->getValue()) {
            m_title = params.getSubParamByName("Block name")->getValue();
            emit appearanceChanged();
        }


        if(m_params.getSubParamByName("Variable name")->getValue().isEmpty()) {
            m_params.getSubParamByName("Variable name")->setState(TConfigParam::TState::TError, "Varaible name cannot be empty.");
        }

        if(m_subtitle != params.getSubParamByName("Variable name")->getValue()) {
            m_subtitle = params.getSubParamByName("Variable name")->getValue();
            emit appearanceChanged();
        }

        if(m_params.getState(true) == TConfigParam::TState::TError) {
            setState(TState::TError, tr("Block configuration contains errors!"));
        }
        else {
            resetState();
        }

        return m_params;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        // purposefully left blank
        return QHash<TScenarioItemPort *, QByteArray>();
    }
};

#endif // TSCENARIOVARIABLEWRITEITEM_H
