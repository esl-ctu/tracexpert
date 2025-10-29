#ifndef TSCENARIOANALDEVICEITEM_H
#define TSCENARIOANALDEVICEITEM_H

#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioAnalDeviceItem class represents a block that represents an Analytic Device.
 *
 * The class represents a block that represents an Analytic Device.
 * It is a base class for Analytic Device Read and Write blocks, it cannot be used on its own.
 * In its basic form, is a block with one input flow port and two output flow ports.
 * The output flow ports are "done" and "error" - they are selected based on the success of the Analytic Device operation.
 * The Analytic Device can be selected in the configuration. It is selected from the list of available Analytic Devices in the selected Component.
 * The Analytic Device can be configured with pre-init and post-init parameters that can be set once,
 * before the scenario is launched, before the block is first executed, or each time the block is executed.
 *
 */
class TScenarioAnalDeviceItem : public TScenarioItem {

public:
    enum { TItemClass = 130 };
    int itemClass() const override { return TItemClass; }

    TScenarioAnalDeviceItem(QString name, QString description) : TScenarioItem(name, description) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));

        m_params = TConfigParam("Block configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Component", "", TConfigParam::TType::TEnum, tr("Component that the Analytic Device belongs to."), false));
        m_params.addSubParam(TConfigParam("Analytic Device", "", TConfigParam::TType::TEnum, tr("The Analytic Device this block represents."), false));

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

        TConfigParam paramConf("Analytic Device configuration", "", TConfigParam::TType::TDummy, tr("The Analytic Device pre-init and post-init configuration."), false);
        paramConf.setState(TConfigParam::TState::TInfo, tr("If you change pre-init params here, keep in mind you need to <b>reinitialize the device with these parameters now</b> to see updated options for post-init params and further configuration."));
        m_params.addSubParam(paramConf);


        // item has to be initialized, otherwise it cannot be used
        m_subtitle = "no Analytic Device selected";
        setState(TState::TError, tr("Block configuration contains errors!"));
    }

    bool supportsDirectExecution() const override {
        return false;
    }

    TScenarioItem * copy() const override {
        return new TScenarioAnalDeviceItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/analytic.png";
    }

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Component") ||
                isParamValueDifferent(newParams, m_params, "Analytic Device");
    }

    void updateParams(bool paramValuesChanged) override {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");
        componentParam->clearEnumValues();
        componentParam->resetState();
        componentParam->addEnumValue("");

        TConfigParam * deviceParam = m_params.getSubParamByName("Analytic Device");
        deviceParam->clearEnumValues();
        deviceParam->resetState();
        deviceParam->addEnumValue("");

        TConfigParam * configParam = m_params.getSubParamByName("Analytic Device configuration");

        TDeviceModel * selectedDeviceModel = nullptr;
        TComponentModel * selectedComponentModel = nullptr;

        for(int i = 0; i < m_projectModel->componentContainer()->count(); i++) {
            TComponentModel * componentModel = m_projectModel->componentContainer()->at(i);

            if(!componentModel->canAddAnalDevice() && componentModel->analDeviceCount() == 0) {
                continue;
            }

            componentParam->addEnumValue(componentModel->name());
        }

        selectedComponentModel = m_projectModel->componentContainer()->getByName(componentParam->getValue());
        if(!selectedComponentModel) {
            componentParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }

        if(selectedComponentModel) {
            for(int i = 0; i < selectedComponentModel->analDeviceCount(); i++) {
                deviceParam->addEnumValue(selectedComponentModel->analDevice(i)->name());
            }

            selectedDeviceModel = selectedComponentModel->analDeviceContainer()->getByName(deviceParam->getValue());
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

        params.getSubParamByName("Analytic Device", &iok);
        if(!iok) return false;

        params.getSubParamByName("Set pre-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("Set post-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("Analytic Device configuration", &iok);
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
            setState(TState::TOk);
        }

        return m_params;
    }

    bool prepare() override {
        resetState();

        m_analDeviceModel = getAnalDeviceModel();
        m_isFirstBlockExecution = true;

        if(!m_analDeviceModel) {
            setState(TState::TError, tr("Failed to obtain selected Analytic Device, is it available?"));
            return false;
        }

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

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
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
        TConfigParam * preInitParams = m_params.getSubParamByName("Analytic Device configuration")->getSubParamByName("pre-init params");

        if(!preInitParams) {
            qWarning("No pre-init params were found, could not be set.");
            setState(TState::TError, tr("No pre-init params were found, could not be set."));
            return;
        }

        if(m_analDeviceModel->isInit() && !m_analDeviceModel->deInit()) {
            qWarning("Could not deinitialize Analytic Device to set pre-init params.");
            setState(TState::TError, tr("Could not deinitialize Analytic Device to set pre-init params."));
            return;
        }

        TConfigParam params = m_analDeviceModel->setPreInitParams(*preInitParams);

        if(!m_analDeviceModel->init()) {
            qWarning("Could not initialize Analytic Device after setting pre-init params.");
            setState(TState::TError, tr("Could not initialize Analytic Device after setting pre-init params."));
            return;
        }

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set pre-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString("[%1] Pre-init params were set").arg(m_analDeviceModel->name()));
    }

    void setPostInitParams() {
        TConfigParam * postInitParams = m_params.getSubParamByName("Analytic Device configuration")->getSubParamByName("post-init params");

        if(!postInitParams) {
            qWarning("No post-init params were found, could not be set.");
            setState(TState::TError, tr("No post-init params were found, could not be set."));
            return;
        }

        TConfigParam params = m_analDeviceModel->setPostInitParams(*postInitParams);

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set post-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString("[%1] Post-init params were set").arg(m_analDeviceModel->name()));
    }

    TAnalDeviceModel * getAnalDeviceModel() {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");
        TComponentModel * componentModel = m_projectModel->componentContainer()->getByName(componentParam->getValue());

        if(!componentModel) {
            return nullptr;
        }

        TConfigParam * analDeviceParam = m_params.getSubParamByName("Analytic Device");
        TAnalDeviceModel * analDeviceModel = componentModel->analDeviceContainer()->getByName(analDeviceParam->getValue());

        if(!analDeviceModel || !analDeviceModel->isAvailable()) {
            return nullptr;
        }

        return analDeviceModel;
    }

    TScenarioItemPort * getPreferredOutputFlowPort() override {
        return m_state == TState::TRuntimeError ? this->getItemPortByName("flowOutError") : this->getItemPortByName("flowOut");
    }

protected:
    TAnalDeviceModel * m_analDeviceModel = nullptr;
    bool m_isFirstBlockExecution;
};

#endif // TSCENARIOANALDEVICEITEM_H
