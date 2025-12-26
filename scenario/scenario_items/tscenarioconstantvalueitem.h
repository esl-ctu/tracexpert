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

#ifndef TSCENARIOCONSTANTVALUEITEM_H
#define TSCENARIOCONSTANTVALUEITEM_H

#include <QIODevice>
#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioConstantValueItem class represents a block that outputs a constant value.
 *
 * The class represents a block that outputs a constant value. It is a block with one output data port.
 * The value of the constant can be set in the configuration.
 * The data type of the constant can be set in the configuration.
 *
 */
class TScenarioConstantValueItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioConstantValueItem;
    }

    TScenarioConstantValueItem() : TScenarioItem(tr("Constant"), tr("This block represents a constant value.")) {
        setType(TItemAppearance::TEmbeddedSubtitle);
        addDataOutputPort("dataOut", "", "", "[selected data type]");

        TConfigParam typeParam("Data type", "string", TConfigParam::TType::TEnum, tr("Data type of constant value."), false);
        typeParam.addEnumValue("string");
        typeParam.addEnumValue("byte array");
        typeParam.addEnumValue("integer");
        typeParam.addEnumValue("unsigned integer");
        typeParam.addEnumValue("short");
        typeParam.addEnumValue("unsigned short");
        typeParam.addEnumValue("long long");
        typeParam.addEnumValue("unsigned long long");
        typeParam.addEnumValue("real");
        typeParam.addEnumValue("bool");

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Block name", "Constant", TConfigParam::TType::TString, tr("Display name of the block."), false));
        m_params.addSubParam(typeParam);
        m_params.addSubParam(TConfigParam("Value", "Hello, world!", TConfigParam::TType::TString, tr("Value of constant."), false));

        m_subtitle = "Hello, world!";
    }

    TScenarioItem * copy() const override {
        return new TScenarioConstantValueItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/constant.png";
    }

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Data type");
    }

    void updateParams(bool paramValuesChanged) override {
        QString oldValue = m_params.getSubParamByName("Value")->getValue();
        m_params.removeSubParam("Value");

        QString defaultValue;
        TConfigParam::TType newType = TConfigParam::TType::TString;
        QString type = m_params.getSubParamByName("Data type")->getValue();

        if(type == "byte array") {
            newType = TConfigParam::TType::TByteArray;
            defaultValue = "deadbeef";
        }
        if(type == "integer") {
            newType = TConfigParam::TType::TInt;
            defaultValue = "0";
        }
        else if(type == "unsigned integer") {
            newType = TConfigParam::TType::TUInt;
            defaultValue = "0";
        }
        else if(type == "short") {
            newType = TConfigParam::TType::TShort;
            defaultValue = "0";
        }
        else if(type == "unsigned short") {
            newType = TConfigParam::TType::TUShort;
            defaultValue = "0";
        }
        else if(type == "long long") {
            newType = TConfigParam::TType::TLongLong;
            defaultValue = "0";
        }
        else if(type == "unsigned long long") {
            newType = TConfigParam::TType::TULongLong;
            defaultValue = "0";
        }
        else if(type == "real") {
            newType = TConfigParam::TType::TReal;
            defaultValue = "0";
        }
        else if(type == "bool") {
            newType = TConfigParam::TType::TBool;
            defaultValue = "true";
        }

        // TODO: create a connection to be able to update the type hint and show it in the UI
        // getItemPortByName("dataOut")->setDataTypeHint(QString("[%1]").arg(type));

        TConfigParam newValueParam("Value", "", newType, tr("Value of constant."), false);

        bool iok;
        newValueParam.setValue(oldValue, &iok);
        if(!iok) {
            newValueParam.setValue(defaultValue);
        }

        m_params.addSubParam(newValueParam);
    }

    bool validateParamsStructure(TConfigParam params) override {
        bool iok = false;

        params.getSubParamByName("Block name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Data type", &iok);
        if(!iok) return false;

        params.getSubParamByName("Value", &iok);
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

        TConfigParam * valueParam = m_params.getSubParamByName("Value");
        valueParam->resetState();

        bool iok;
        valueParam->setValue(valueParam->getValue(), &iok);

        if(!iok) {
            valueParam->setState(TConfigParam::TState::TError, tr("Invalid value."));
        }

        m_title = params.getSubParamByName("Block name")->getValue();
        m_subtitle = valueParam->getValue();

        QString type = m_params.getSubParamByName("Data type")->getValue();
        if(type == "byte array") {
             m_subtitle = "0x" + m_subtitle;
        }

        emit appearanceChanged();
        return m_params;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        QString stringValue = m_params.getSubParamByName("Value")->getValue();

        QByteArray byteValue;
        QDataStream byteStream(&byteValue, QIODevice::WriteOnly);
        byteStream.setByteOrder(QDataStream::LittleEndian);

        QString type = m_params.getSubParamByName("Data type")->getValue();
        if(type == "integer") {
            byteStream << stringValue.toInt();
        }
        else if(type == "unsigned integer") {
            byteStream << stringValue.toUInt();
        }
        else if(type == "short") {
            byteStream << stringValue.toShort();
        }
        else if(type == "unsigned short") {
            byteStream << stringValue.toUShort();
        }
        else if(type == "long long") {
            byteStream << stringValue.toLongLong();
        }
        else if(type == "unsigned long long") {
            byteStream << stringValue.toULongLong();
        }
        else if(type == "real") {
            byteStream << stringValue.toDouble();
        }
        else if(type == "byte array") {
            // Bugfix - extra bytes are prepended by QDataStream to arrays
            // --> skip over QDataStream and save directly to byteValue
            byteValue = QByteArray::fromHex(stringValue.toUtf8());
        }
        else if(type == "bool") {
            byteStream << (bool)(stringValue.toLower() == "true");
        }
        else {
            // Bugfix - extra bytes are prepended by QDataStream to arrays
            // --> skip over QDataStream and save directly to byteValue
            byteValue = stringValue.toUtf8();
        }

        QHash<TScenarioItemPort *, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), byteValue);

        return outputData;
    }

protected:
    TConfigParam * m_currentValueParam;

};

#endif // TSCENARIOCONSTANTVALUEITEM_H
