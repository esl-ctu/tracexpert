#ifndef TSCENARIOANALDEVICEACTIONITEM_H
#define TSCENARIOANALDEVICEACTIONITEM_H

#include "tscenarioanaldeviceitem.h"

class TScenarioAnalDeviceActionItem : public TScenarioAnalDeviceItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioAnalDeviceActionItem;
    }

    TScenarioAnalDeviceActionItem() : TScenarioAnalDeviceItem(tr("Analytic Device: action"), tr("This block performs action on a selected Analytic Device.")) {
        TConfigParam actionParam("Action to execute", "", TConfigParam::TType::TEnum, tr("Select the action to execute."), false);
        m_params.addSubParam(actionParam);
    }

    TScenarioItem * copy() const override {
        return new TScenarioAnalDeviceActionItem(*this);
    }

    bool validateParamsStructure(TConfigParam params) override {
        if(!TScenarioAnalDeviceItem::validateParamsStructure(params)) {
            return false;
        }

        bool iok;
        params.getSubParamByName("Action to execute", &iok);

        return iok;
    }

    void updateParams(bool paramValuesChanged) override {
        TScenarioAnalDeviceItem::updateParams(paramValuesChanged);

        TAnalDeviceModel * deviceModel = getDeviceModel();

        if(deviceModel && paramValuesChanged) {
            TConfigParam * actionParam = m_params.getSubParamByName("Action to execute");
            actionParam->clearEnumValues();
            actionParam->resetState();

            for(TAnalActionModel * actionModel : deviceModel->actionModels()) {
                actionParam->addEnumValue(actionModel->name());
            }
        }
    }

    TAnalActionModel * getAnalDeviceActionModel() {
        TConfigParam * actionParam = m_params.getSubParamByName("Action to execute");

        if(!m_deviceModel) {
            return nullptr;
        }

        for(TAnalActionModel * actionModel : m_deviceModel->actionModels()) {
            if(actionModel->name() == actionParam->getValue()) {
                return actionModel;
            }
        }

        return nullptr;
    }

    void actionStarted() {
        log(QString("[%1] Action %2 started.").arg(m_deviceModel->name(), m_analActionModel->name()));
        m_isActionRunning = true;
    }

    void actionFinished() {
        disconnect(m_analActionModel, nullptr, this, nullptr);

        log(QString("[%1] Action %2 done.").arg(m_deviceModel->name(), m_analActionModel->name()));
        m_isActionRunning = false;

        emit executionFinished();
    }

    bool cleanup() override {
        TScenarioComponentItem::cleanup();
        disconnect(m_analActionModel, nullptr, this, nullptr);
        return true;
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        m_analActionModel = getAnalDeviceActionModel();

        if(!m_analActionModel) {
            setState(TState::TRuntimeError, tr("The action was not found."));
            emit executionFinished();
            return;
        }

        connect(m_analActionModel, &TAnalActionModel::started, this, &TScenarioAnalDeviceActionItem::actionStarted);
        connect(m_analActionModel, &TAnalActionModel::finished, this, &TScenarioAnalDeviceActionItem::actionFinished);

        emit m_analActionModel->run();        
    }

    void stopExecution() override {
        if(m_isActionRunning) {
            emit m_analActionModel->abort();
        }
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioAnalDeviceItem::setParams(params);

        m_title = "";
        m_subtitle = "no Analytic Device selected";
        if(m_params.getState(true) != TConfigParam::TState::TError) {
            m_title = m_params.getSubParamByName("Analytic Device")->getValue() + ": action";
            m_subtitle = m_params.getSubParamByName("Action to execute")->getValue();
        }

        emit appearanceChanged();
        return paramsToReturn;
    }

protected:
    bool m_isActionRunning = false;
    TAnalActionModel * m_analActionModel = nullptr;
};


#endif // TSCENARIOANALDEVICEACTIONITEM_H
