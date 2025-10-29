#ifndef TSCENARIOIODEVICEITEM_H
#define TSCENARIOIODEVICEITEM_H

#include "../tscenarioitem.h"

/*!
 * \brief The TScenarioIODeviceItem class represents a block that represents an IO Device.
 *
 * The class represents a block that represents an IO Device.
 * It is a base class for IO Device Read and Write blocks, it cannot be used on its own.
 * In its basic form, is a block with one input flow port and two output flow ports.
 * The output flow ports are "done" and "error" - they are selected based on the success of the IO Device operation.
 * The IO Device can be selected in the configuration. It is selected from the list of available IO Devices in the selected Component.
 * The IO Device can be configured with pre-init and post-init parameters that can be set once,
 * before the scenario is launched, before the block is first executed, or each time the block is executed.
 *
 */
class TScenarioIODeviceItem : public TScenarioItem {

public:
    enum { TItemClass = 30 };
    int itemClass() const override { return TItemClass; }

    TScenarioIODeviceItem(QString name, QString description) : TScenarioItem(name, description) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));

        m_params = TConfigParam("Block configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Component", "", TConfigParam::TType::TEnum, tr("Component that the IO Device belongs to."), false));
        m_params.addSubParam(TConfigParam("IO Device", "", TConfigParam::TType::TEnum, tr("The IO Device this block represents."), false));

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

        m_params.addSubParam(TConfigParam("IO Device configuration", "", TConfigParam::TType::TDummy, tr("The IO Device pre-init and post-init configuration."), false));

        // item has to be initialized, otherwise it cannot be used
        m_subtitle = "no IO device selected";
        setState(TState::TError, tr("Block configuration contains errors!"));
    }

    bool supportsDirectExecution() const override {
        return false;
    }

    TScenarioItem * copy() const override {
        return new TScenarioIODeviceItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/file.png";
    }

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Component") ||
                isParamValueDifferent(newParams, m_params, "IO Device");
    }

    void updateParams(bool paramValuesChanged) override {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");
        componentParam->clearEnumValues();
        componentParam->resetState();

        int componentCount = m_projectModel->componentContainer()->count();
        int selectedComponentIndex = -1;
        for(int i = 0; i < componentCount; i++) {
            if(!m_projectModel->componentContainer()->at(i)->canAddIODevice() &&
                m_projectModel->componentContainer()->at(i)->IODeviceCount() == 0)
                continue;

            if(selectedComponentIndex == -1) {
                selectedComponentIndex = i;
            }

            QString componentName = m_projectModel->componentContainer()->at(i)->name();
            componentParam->addEnumValue(componentName);

            if(componentName == componentParam->getValue()) {
                selectedComponentIndex = i;
            }
        }

        if(selectedComponentIndex == -1) {
            componentParam->setValue("");
            componentParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }

        TConfigParam * IODeviceParam = m_params.getSubParamByName("IO Device");
        IODeviceParam->clearEnumValues();

        int selectedIODeviceIndex = -1;
        if(selectedComponentIndex > -1) {
            IODeviceParam->resetState();
            TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);

            int IODeviceCount = componentModel->IODeviceCount();
            selectedIODeviceIndex = IODeviceCount > 0 ? 0 : -1;
            for(int i = 0; i < IODeviceCount; i++) {
                QString IODeviceName = componentModel->IODeviceContainer()->at(i)->name();
                IODeviceParam->addEnumValue(IODeviceName);

                if(IODeviceName == IODeviceParam->getValue()) {
                    selectedIODeviceIndex = i;
                }
            }

            if(selectedIODeviceIndex > -1) {
                IODeviceParam->setValue(componentModel->IODeviceContainer()->at(selectedIODeviceIndex)->name());
            }
        }

        if(selectedIODeviceIndex == -1) {
            IODeviceParam->setValue("");
            IODeviceParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }

        TConfigParam * configParam = m_params.getSubParamByName("IO Device configuration");

        if(selectedIODeviceIndex > -1 && paramValuesChanged) {
            configParam->clearSubParams();

            TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);
            TIODeviceModel * IODeviceModel = componentModel->IODeviceContainer()->at(selectedIODeviceIndex);

            TConfigParam preInitParams = IODeviceModel->preInitParams();
            preInitParams.setName("pre-init params");

            configParam->addSubParam(preInitParams);

            TConfigParam postInitParams = IODeviceModel->postInitParams();
            postInitParams.setName("post-init params");

            configParam->addSubParam(postInitParams);
        }
    }

    bool validateParamsStructure(TConfigParam params) override {
        bool iok = false;

        params.getSubParamByName("Component", &iok);
        if(!iok) return false;

        params.getSubParamByName("IO Device", &iok);
        if(!iok) return false;

        params.getSubParamByName("Set pre-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("Set post-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("IO Device configuration", &iok);
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

        m_IODeviceModel = getIODeviceModel();
        m_isFirstBlockExecution = true;

        if(!m_IODeviceModel) {
            setState(TState::TError, tr("Failed to obtain selected IO device, is it available?"));
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
        TConfigParam * preInitParams = m_params.getSubParamByName("IO Device configuration")->getSubParamByName("pre-init params");

        if(!preInitParams) {
            qWarning("No pre-init params were found, could not be set.");
            setState(TState::TError, tr("No pre-init params were found, could not be set."));
            return;
        }

        if(m_IODeviceModel->isInit() && !m_IODeviceModel->deInit()) {
            qWarning("Could not deinitialize IO device to set pre-init params.");
            setState(TState::TError, tr("Could not deinitialize IO device to set pre-init params."));
            return;
        }

        TConfigParam params = m_IODeviceModel->setPreInitParams(*preInitParams);

        if(!m_IODeviceModel->init()) {
            qWarning("Could not initialize IO device after setting pre-init params.");
            setState(TState::TError, tr("Could not initialize IO device after setting pre-init params."));
            return;
        }

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set pre-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString("[%1] Pre-init params were set").arg(m_IODeviceModel->name()));
    }

    void setPostInitParams() {
        TConfigParam * postInitParams = m_params.getSubParamByName("IO Device configuration")->getSubParamByName("post-init params");

        if(!postInitParams) {
            qWarning("No post-init params were found, could not be set.");
            setState(TState::TError, tr("No post-init params were found, could not be set."));
            return;
        }

        TConfigParam params = m_IODeviceModel->setPostInitParams(*postInitParams);

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set post-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString("[%1] Post-init params were set").arg(m_IODeviceModel->name()));
    }

    TIODeviceModel * getIODeviceModel() {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");

        int componentCount = m_projectModel->componentContainer()->count();
        int selectedComponentIndex = -1;
        for(int i = 0; i < componentCount; i++) {
            QString componentName = m_projectModel->componentContainer()->at(i)->name();
            if(componentName == componentParam->getValue()) {
                selectedComponentIndex = i;
            }
        }

        TConfigParam * IODeviceParam = m_params.getSubParamByName("IO Device");

        int selectedIODeviceIndex = -1;
        if(selectedComponentIndex > -1) {
            TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);

            int IODeviceCount = componentModel->IODeviceCount();
            for(int i = 0; i < IODeviceCount; i++) {
                QString IODeviceName = componentModel->IODeviceContainer()->at(i)->name();
                if(IODeviceName == IODeviceParam->getValue()) {
                    selectedIODeviceIndex = i;
                }
            }
        }

        if(selectedIODeviceIndex < 0) {
            return nullptr;
        }

        TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);
        TIODeviceModel * IODeviceModel = componentModel->IODeviceContainer()->at(selectedIODeviceIndex);

        if(!IODeviceModel->isAvailable()) {
            return nullptr;
        }

        return IODeviceModel;
    }

    TScenarioItemPort * getPreferredOutputFlowPort() override {
        return m_state == TState::TRuntimeError ? this->getItemPortByName("flowOutError") : this->getItemPortByName("flowOut");
    }

protected:
    TIODeviceModel * m_IODeviceModel = nullptr;
    bool m_isFirstBlockExecution;
};

#endif // TSCENARIOIODEVICEITEM_H
