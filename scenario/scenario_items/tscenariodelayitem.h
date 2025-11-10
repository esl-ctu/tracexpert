
#ifndef TSCENARIODELAYITEM_H
#define TSCENARIODELAYITEM_H

#include <QIODevice>
#include <qtimer.h>
#include "../tscenarioitem.h"

/*!
 */
class TScenarioDelayItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioDelayItem;
    }

    TScenarioDelayItem() : TScenarioItem(tr("Delay"), tr("This block represents a delay in execution.")), m_delayTimer(nullptr) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port after set delay."));

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Block name", "Delay", TConfigParam::TType::TString, tr("Display name of the block."), false));
        m_params.addSubParam(TConfigParam("Length", "3", TConfigParam::TType::TReal, tr("Length of delay in seconds."), false));

        m_subtitle = "3s";
    }

    const QString getIconResourcePath() const override {
        return ":/icons/delay.png";
    }

    TScenarioItem * copy() const override {
        return new TScenarioDelayItem(*this);
    }

    bool validateParamsStructure(TConfigParam params) {
        bool iok = false;

        params.getSubParamByName("Block name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Length", &iok);
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

        TConfigParam * valueParam = m_params.getSubParamByName("Length");

        bool iok;
        valueParam->setValue(valueParam->getValue(), &iok);

        if(!iok) {
            valueParam->setState(TConfigParam::TState::TError, tr("Invalid value."));
        }

        m_subtitle = QString("%1s").arg(valueParam->getValue());
        emit appearanceChanged();

        return m_params;
    }

    bool supportsDirectExecution() const override {
        return false;
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {

        bool iok;
        double delay = m_params.getSubParamByName("Length")->getValue().toDouble(&iok);

        if(!iok) {
            log(tr("Failed to delay: length invalid..."), TLogLevel::TError);
            emit executionFinished();
            return;
        }

        if(!m_delayTimer) {
            log(tr("Starting delay..."));

            m_delayTimer = new QTimer();
            QObject::connect(m_delayTimer, &QTimer::timeout, this, &TScenarioDelayItem::delayElapsed);
            m_delayTimer->setSingleShot(true);
            m_delayTimer->start(delay*1000);
        }
        else {
            log(tr("Failed to delay: could not start timer..."), TLogLevel::TError);
            emit executionFinished();
            return;
        }
    }

    bool cleanup() override {
        if(m_delayTimer) {
            m_delayTimer->stop();

            delete m_delayTimer;
            m_delayTimer = nullptr;
        }

        return true;
    }

    void delayElapsed() {
        if(m_delayTimer) {
            delete m_delayTimer;
            m_delayTimer = nullptr;
        }

        log(tr("Delay over."));
        emit executionFinished();
        return;
    }

private:
    QTimer * m_delayTimer;
};

#endif // TSCENARIODELAYITEM_H
