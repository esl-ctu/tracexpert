#ifndef TSCENARIOCOMPONENTITEM_H
#define TSCENARIOCOMPONENTITEM_H

#include "../tscenarioitem.h"

template <typename T> class TScenarioComponentItem : public TScenarioItem {

public:
    TScenarioComponentItem(QString deviceName, QString name, QString description) : TScenarioItem(name, description), m_deviceName(deviceName) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));

        m_params = TConfigParam("Block configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Component", "", TConfigParam::TType::TEnum, QString(tr("Component that the %1 belongs to.")).arg(deviceName), false));
        m_params.addSubParam(TConfigParam(deviceName, "", TConfigParam::TType::TEnum, QString(tr("The %1 this block represents.")).arg(deviceName), false));

        TConfigParam preInitParamConf("Set pre-init params", "", TConfigParam::TType::TEnum, tr("Select if and when the configured pre-init parameters should be set."), false);
        preInitParamConf.addEnumValue("Never");
        preInitParamConf.addEnumValue("Once; when the scenario is launched");
        preInitParamConf.addEnumValue("Once; before this block is first executed");
        preInitParamConf.addEnumValue("Each time this block is executed");
        m_params.addSubParam(preInitParamConf);

        TConfigParam postInitParamConf("Set post-init params", "", TConfigParam::TType::TEnum, tr("Select if and when the configured post-init parameters should be set."), false);
        postInitParamConf.addEnumValue("Never");
        postInitParamConf.addEnumValue("Once; when the scenario is launched");
        postInitParamConf.addEnumValue("Once; before this block is first executed");
        postInitParamConf.addEnumValue("Each time this block is executed");
        m_params.addSubParam(postInitParamConf);

        TConfigParam paramConf("Device configuration", "", TConfigParam::TType::TDummy, tr("The device's pre-init and post-init configuration."), false);
        paramConf.setState(TConfigParam::TState::TInfo, tr("If you change pre-init params here, keep in mind you need to <b>reinitialize the device with these parameters now</b> to see updated options for post-init params and further configuration."));
        m_params.addSubParam(paramConf);

        m_allowedDynamicParamNames << "pre-init params*";
        m_allowedDynamicParamNames << "post-init params*";

        // item has to be initialized, otherwise it cannot be used
        m_subtitle = QString(tr("no %1 selected")).arg(deviceName);
        setState(TState::TError, tr("Block configuration contains errors!"));
    }

    TScenarioItem * copy() const override {
        return new TScenarioComponentItem(*this);
    }

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Component") ||
               isParamValueDifferent(newParams, m_params, m_deviceName);
    }

    virtual bool isCompatibleComponent(TComponentModel * componentModel) {
        return false;
    }

    virtual int deviceCount(TComponentModel * componentModel) {
        return 0;
    }

    virtual T * deviceByIndex(TComponentModel * componentModel, int index) {
        return nullptr;
    }

    virtual T * deviceByName(TComponentModel * componentModel, const QString & name) {
        return nullptr;
    }

    void updateParams(bool paramValuesChanged) override {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");
        componentParam->clearEnumValues();
        componentParam->resetState();
        componentParam->addEnumValue("");

        TConfigParam * deviceParam = m_params.getSubParamByName(m_deviceName);
        deviceParam->clearEnumValues();
        deviceParam->resetState();
        deviceParam->addEnumValue("");

        TConfigParam * configParam = m_params.getSubParamByName("Device configuration");

        TDeviceModel * selectedDeviceModel = nullptr;
        TComponentModel * selectedComponentModel = nullptr;

        for(int i = 0; i < m_projectModel->componentContainer()->count(); i++) {
            TComponentModel * componentModel = m_projectModel->componentContainer()->at(i);

            if(!componentModel->isInit() || !isCompatibleComponent(componentModel)) {
                continue;
            }

            componentParam->addEnumValue(componentModel->name());
        }

        selectedComponentModel = m_projectModel->componentContainer()->getByName(componentParam->getValue());
        if(!selectedComponentModel) {
            componentParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }

        if(selectedComponentModel) {
            for(int i = 0; i < deviceCount(selectedComponentModel); i++) {
                deviceParam->addEnumValue(deviceByIndex(selectedComponentModel, i)->name());
            }

            selectedDeviceModel = deviceByName(selectedComponentModel, deviceParam->getValue());
        }

        if(!selectedDeviceModel) {
            deviceParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }

        if(selectedDeviceModel && paramValuesChanged) {
            configParam->clearSubParams();

            TConfigParam preInitParams = selectedDeviceModel->preInitParams();
            preInitParams.setName("pre-init params");

            configParam->addSubParam(preInitParams);

            TConfigParam postInitParams = selectedDeviceModel->postInitParams();
            postInitParams.setName("post-init params");

            configParam->addSubParam(postInitParams);
        }
    }

    bool validateParamsStructure(TConfigParam params) override {
        bool iok = false;

        params.getSubParamByName("Component", &iok);
        if(!iok) return false;

        params.getSubParamByName(m_deviceName, &iok);
        if(!iok) return false;

        params.getSubParamByName("Set pre-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("Set post-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("Device configuration", &iok);
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

    bool checkAndSetInitParamsAtPreparation() {
        TConfigParam * preInitParamConf = m_params.getSubParamByName("Set pre-init params");
        if(preInitParamConf->getValue() == "Once; when the scenario is launched") {
            setPreInitParams();
        }

        if(getState() == TState::TError) {
            return false;
        }

        TConfigParam * postInitParamConf = m_params.getSubParamByName("Set post-init params");
        if(postInitParamConf->getValue() == "Once; when the scenario is launched") {
            setPostInitParams();
        }

        if(getState() == TState::TError) {
            return false;
        }

        return true;
    }

    bool prepare() override {
        resetState();

        m_deviceModel = getDeviceModel();
        m_isFirstBlockExecution = true;

        if(!m_deviceModel) {
            setState(TState::TError, tr("Failed to obtain selected device, is it available?"));
            return false;
        }

        if(!checkAndSetInitParamsAtPreparation()) {
            return false;
        }

        return true;
    }

    bool cleanup() override {
        // make sure no signals are received after scenario stops
        if(m_deviceModel) {
            disconnect(m_deviceModel, nullptr, this, nullptr);
            m_deviceModel = nullptr;
        }
        return true;
    }

    void checkAndSetInitParamsBeforeExecution() {
        TConfigParam * preInitParamConf = m_params.getSubParamByName("Set pre-init params");
        if(preInitParamConf->getValue() == "Each time this block is executed" ||
            (preInitParamConf->getValue() == "Once; before this block is first executed" && m_isFirstBlockExecution)) {
            setPreInitParams();
        }

        TConfigParam * postInitParamConf = m_params.getSubParamByName("Set post-init params");
        if(postInitParamConf->getValue() == "Each time this block is executed" ||
            (postInitParamConf->getValue() == "Once; before this block is first executed" && m_isFirstBlockExecution)) {
            setPostInitParams();
        }

        m_isFirstBlockExecution = false;
    }

    void setPreInitParams() {
        TConfigParam * preInitParams = m_params.getSubParamByName("Device configuration")->getSubParamByName("pre-init params");

        if(!preInitParams) {
            qWarning("No pre-init params were found, could not be set.");
            setState(TState::TError, tr("No pre-init params were found, could not be set."));
            return;
        }

        if(m_deviceModel->isInit() && !m_deviceModel->deInit()) {
            qWarning("Could not deinitialize device to set pre-init params.");
            setState(TState::TError, tr("Could not deinitialize device to set pre-init params."));
            return;
        }

        TConfigParam params = m_deviceModel->setPreInitParams(*preInitParams);

        if(!m_deviceModel->init()) {
            qWarning("Could not initialize device after setting pre-init params.");
            setState(TState::TError, tr("Could not initialize device after setting pre-init params."));
            return;
        }

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set pre-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString("[%1] Pre-init params were set").arg(m_deviceModel->name()));
    }

    void setPostInitParams() {
        TConfigParam * postInitParams = m_params.getSubParamByName("Device configuration")->getSubParamByName("post-init params");

        if(!postInitParams) {
            qWarning("No post-init params were found, could not be set.");
            setState(TState::TError, tr("No post-init params were found, could not be set."));
            return;
        }

        TConfigParam params = m_deviceModel->setPostInitParams(*postInitParams);

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set post-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString("[%1] Post-init params were set").arg(m_deviceModel->name()));
    }

    T * getDeviceModel() {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");
        TComponentModel * componentModel = m_projectModel->componentContainer()->getByName(componentParam->getValue());

        if(!componentModel) {
            return nullptr;
        }

        TConfigParam * deviceParam = m_params.getSubParamByName(m_deviceName);
        T * deviceModel = deviceByName(componentModel, deviceParam->getValue());

        if(!deviceModel || !deviceModel->isAvailable()) {
            return nullptr;
        }

        return deviceModel;
    }

    TScenarioItemPort * getPreferredOutputFlowPort() override {
        return m_state == TState::TRuntimeError ? this->getItemPortByName("flowOutError") : this->getItemPortByName("flowOut");
    }

protected:
    QString m_deviceName;

    T * m_deviceModel = nullptr;
    bool m_isFirstBlockExecution;
};


#endif // TSCENARIOCOMPONENTITEM_H
