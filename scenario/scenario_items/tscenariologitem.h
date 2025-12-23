#ifndef TSCENARIOLOGITEM_H
#define TSCENARIOLOGITEM_H

#include <QString>
#include <QList>
#include <QDataStream>

#include "../tscenarioitem.h"

#define SCENARIO_LOG_ENTRY_SIZE_LIMIT 512

/*!
 * \brief The TScenarioLogItem class represents a block that logs data input to scenario run log.
 *
 * The class represents a block that logs data input to scenario run log.
 *
 */
class TScenarioLogItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioLogItem;
    }

    TScenarioLogItem() : TScenarioItem(tr("Logger"), tr("This block logs data input to scenario run log.")) {
        addFlowInputPort("flowIn");
        addDataInputPort("dataIn", "", tr("Data pased through this port will be put into the scenario run log."), "[any]");
        addFlowOutputPort("flowOut");

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        TConfigParam modeParam("Log format", "string", TConfigParam::TType::TEnum, tr("Log format - string or hex."), false);
        modeParam.addEnumValue("string");
        modeParam.addEnumValue("hex");
        m_params.addSubParam(modeParam);
    }

    TScenarioItem * copy() const override {
        return new TScenarioLogItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/log.png";
    }

    TConfigParam setParams(TConfigParam params) override {

        bool iok = false;
        params.getSubParamByName("Log format", &iok);

        if(!iok) {
            params.setState(TConfigParam::TState::TError, "Wrong structure of the pre-init params.");
            return params;
        }

        m_params = params;
        m_params.resetState(true);

        return m_params;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        QByteArray dataToLog = dataInputValues.value(getItemPortByName("dataIn"));

        if(dataToLog.size() > SCENARIO_LOG_ENTRY_SIZE_LIMIT) {
            if(m_params.getSubParamByName("Log format")->getValue() == "hex") {
                log(QString("%1 ... skipping %2 bytes ... %3 (total length %4 bytes)")
                    .arg(dataToLog.first(5).toHex(' '))
                    .arg(dataToLog.length() - 10)
                    .arg(dataToLog.last(5).toHex(' '))
                    .arg(dataToLog.length())
                );
            }
            else {
                log(QString("%1 ... skipping %2 bytes ... %3 (total length %4 bytes)")
                    .arg(dataToLog.first(5))
                    .arg(dataToLog.length() - 10)
                    .arg(dataToLog.last(5))
                    .arg(dataToLog.length())
                );
            }
        }
        else if(m_params.getSubParamByName("Log format")->getValue() == "hex") {
            log(dataToLog.toHex(' '));
        }
        else {
            log(dataToLog);
        }

        return QHash<TScenarioItemPort *, QByteArray>();
    }

};

#endif // TSCENARIOLOGITEM_H

