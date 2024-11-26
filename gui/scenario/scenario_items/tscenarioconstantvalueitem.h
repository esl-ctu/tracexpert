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
    enum { TItemClass = 40 };
    int itemClass() const override { return TItemClass; }

    TScenarioConstantValueItem() : TScenarioItem(tr("Constant"), tr("This block represents a constant value.")) {
        addDataOutputPort("dataOut");

        TConfigParam typeParam("Data type", "string", TConfigParam::TType::TEnum, tr("Data type of constant value."), false);
        typeParam.addEnumValue("string");
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
        m_params.addSubParam(TConfigParam("Value", "", TConfigParam::TType::TString, tr("Value of constant."), false));
    }

    TScenarioItem * copy() const override {
        return new TScenarioConstantValueItem(*this);
    }

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Data type");
    }

    void updateParams(bool paramValuesChanged) override {
        QString oldValue = m_params.getSubParamByName("Value")->getValue();
        m_params.removeSubParam("Value");

        TConfigParam::TType newType = TConfigParam::TType::TString;
        QString type = m_params.getSubParamByName("Data type")->getValue();
        if(type == "integer") {
            newType = TConfigParam::TType::TInt;
        }
        else if(type == "unsigned integer") {
            newType = TConfigParam::TType::TUInt;
        }
        else if(type == "short") {
            newType = TConfigParam::TType::TShort;
        }
        else if(type == "unsigned short") {
            newType = TConfigParam::TType::TUShort;
        }
        else if(type == "long long") {
            newType = TConfigParam::TType::TLongLong;
        }
        else if(type == "unsigned long long") {
            newType = TConfigParam::TType::TULongLong;
        }
        else if(type == "real") {
            newType = TConfigParam::TType::TReal;
        }
        else if(type == "bool") {
            newType = TConfigParam::TType::TBool;
        }

        m_params.addSubParam(TConfigParam("Value", oldValue, newType, tr("Value of constant."), false));
    }

    bool validateParamsStructure(TConfigParam params) {
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

        m_params = params;
        m_params.resetState(true);

        if(m_title != params.getSubParamByName("Block name")->getValue()) {
            m_title = params.getSubParamByName("Block name")->getValue();
            emit appearanceChanged();
        }

        TConfigParam * valueParam = m_params.getSubParamByName("Value");

        bool iok;
        valueParam->setValue(valueParam->getValue(), &iok);

        if(!iok) {
            valueParam->setState(TConfigParam::TState::TError, tr("Invalid value."));
        }

        return m_params;
    }

    QHash<TScenarioItemPort *, QByteArray> executeImmediate(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        QString stringValue = m_params.getSubParamByName("Value")->getValue();

        QByteArray byteValue;
        QDataStream byteStream(&byteValue, QIODevice::WriteOnly);

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
        else if(type == "bool") {
            byteStream << (bool)(stringValue.toLower() == "true");
        }
        else {
            byteStream << stringValue;
        }

        QHash<TScenarioItemPort *, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), byteValue);

        return outputData;
    }

};

#endif // TSCENARIOCONSTANTVALUEITEM_H
