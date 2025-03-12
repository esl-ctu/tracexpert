#ifndef TSCENARIOLOGITEM_H
#define TSCENARIOLOGITEM_H

#include <QString>
#include <QList>
#include <QDataStream>

#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioLogItem class represents a block that logs data input to scenario run log.
 *
 * The class represents a block that logs data input to scenario run log.
 *
 */
class TScenarioLogItem : public TScenarioItem {

public:
    const int SCENARIO_LOG_ENTRY_SIZE_LIMIT = 512;

    enum { TItemClass = 10 };
    int itemClass() const override { return TItemClass; }

    TScenarioLogItem() : TScenarioItem(tr("Logger"), tr("This block logs data input to scenario run log.")) {
        addFlowInputPort("flowIn");
        addDataInputPort("dataIn");
        addFlowOutputPort("flowOut");

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        TConfigParam modeParam("Log format", "raw", TConfigParam::TType::TEnum, tr("Log format - raw or hex."), false);
        modeParam.addEnumValue("raw");
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

    QHash<TScenarioItemPort *, QByteArray> executeImmediate(const QHash<TScenarioItemPort *, QByteArray> & dataInputValues) override {
        QByteArray dataToLog = dataInputValues.value(getItemPortByName("dataIn"));

        if(dataToLog.size() > SCENARIO_LOG_ENTRY_SIZE_LIMIT) {
            log(tr("Passed data is too big to be shown in the log..."), "orange");
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

