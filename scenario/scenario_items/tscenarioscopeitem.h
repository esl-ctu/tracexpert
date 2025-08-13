#ifndef TSCENARIOSCOPEITEM_H
#define TSCENARIOSCOPEITEM_H

#include "../tscenarioitem.h"

/**
 * @brief The TScenarioScopeItem class represents a block that interfaces a selected Scope.
 *
 * The class represents a block that interfaces a selected Scope.
 * It is a block with one input flow port and output flow ports for the number of traces,
 * number of samples per trace, data type of samples, sample data and overvoltage indication.
 *
 * The Scope can be selected in the configuration. It is selected from the list of available Scopes in the selected Component.
 * The Scope can be configured with pre-init and post-init parameters that can be set once,
 * before the scenario is launched, before the block is first executed, or each time the block is executed.
 *
 */
class TScenarioScopeItem : public TScenarioItem {

public:
    enum { TItemClass = 60 };
    int itemClass() const override { return TItemClass; }

    TScenarioScopeItem(QString name, QString description) : TScenarioItem(name, description) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));

        addDataOutputPort("buffers", "data", tr("Byte array with read data."));
        addDataOutputPort("traceCount", "#traces",tr("Number of traces."));
        addDataOutputPort("sampleCount", "#samples", tr("Number of samples per trace."));
        addDataOutputPort("type", "data type", tr("Data type of samples."));
        addDataOutputPort("overvoltage", "overvoltage", tr("Overvoltage indication."));

        m_subtitle = tr("no Oscilloscope selected");

        m_params = TConfigParam("Block configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Component", "", TConfigParam::TType::TEnum, tr("Component that the Oscilloscope belongs to."), false));
        m_params.addSubParam(TConfigParam("Oscilloscope", "", TConfigParam::TType::TEnum, tr("The Oscilloscope this block represents."), false));

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

        m_params.addSubParam(TConfigParam("Oscilloscope configuration", "", TConfigParam::TType::TDummy, tr("The Oscilloscope pre-init and post-init configuration."), false));

        // item has to be initialized, , otherwise it cannot be used
        setState(TState::TError, tr("Block configuration contains errors!"));
    }

    bool supportsImmediateExecution() const override {
        return false;
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/oscilloscope.png";
    }

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Component") ||
               isParamValueDifferent(newParams, m_params, "Oscilloscope");
    }

    void updateParams(bool paramValuesChanged) override {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");
        componentParam->clearEnumValues();
        componentParam->resetState();

        int componentCount = m_projectModel->componentContainer()->count();
        int selectedComponentIndex = -1;
        for(int i = 0; i < componentCount; i++) {
            if(!m_projectModel->componentContainer()->at(i)->canAddScope())
                continue;

            // TODO: put condition into IODevice as well
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

        TConfigParam * scopeParam = m_params.getSubParamByName("Oscilloscope");
        scopeParam->clearEnumValues();

        int selectedScopeIndex = -1;
        if(selectedComponentIndex > -1) {
            scopeParam->resetState();
            TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);

            int scopeCount = componentModel->scopeCount();
            selectedScopeIndex = scopeCount > 0 ? 0 : -1;
            for(int i = 0; i < scopeCount; i++) {
                QString scopeName = componentModel->scopeContainer()->at(i)->name();
                scopeParam->addEnumValue(scopeName);

                if(scopeName == scopeParam->getValue()) {
                    selectedScopeIndex = i;
                }
            }
        }

        if(selectedScopeIndex == -1) {
            scopeParam->setValue("");
            scopeParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }

        TConfigParam * configParam = m_params.getSubParamByName("Oscilloscope configuration");

        if(selectedScopeIndex > -1 && paramValuesChanged) {
            configParam->clearSubParams();

            TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);
            TScopeModel * scopeModel = componentModel->scopeContainer()->at(selectedScopeIndex);

            TConfigParam preInitParams = scopeModel->preInitParams();
            preInitParams.setName("pre-init params");

            configParam->addSubParam(preInitParams);

            TConfigParam postInitParams = scopeModel->postInitParams();
            postInitParams.setName("post-init params");

            configParam->addSubParam(postInitParams);
        }
    }

    bool validateParamsStructure(TConfigParam params) {
        bool iok = false;

        params.getSubParamByName("Component", &iok);
        if(!iok) return false;

        params.getSubParamByName("Oscilloscope", &iok);
        if(!iok) return false;

        params.getSubParamByName("Set pre-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("Set post-init params", &iok);
        if(!iok) return false;

        params.getSubParamByName("Oscilloscope configuration", &iok);
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

        m_title = "";
        m_subtitle = "no Oscilloscope selected";

        if(m_params.getState(true) == TConfigParam::TState::TError) {
            setState(TState::TError, tr("Block configuration contains errors!"));
        }
        else {
            setState(TState::TOk);
            m_title = m_params.getSubParamByName("Component")->getValue();
            m_subtitle = m_params.getSubParamByName("Oscilloscope")->getValue();
        }

        emit appearanceChanged();
        return m_params;
    }

    bool prepare() override {
        m_scopeModel = getScopeModel();
        m_isFirstBlockExecution = true;

        if(!m_scopeModel) {
            setState(TState::TError, tr("Failed to obtain selected Oscilloscope, is it available?"));
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

    void setPreInitParams() {
        TConfigParam * preInitParams = m_params.getSubParamByName("Oscilloscope configuration")->getSubParamByName("pre-init params");

        if(!preInitParams) {
            qWarning("No pre-init params were found, could not be set.");
            setState(TState::TError, tr("No pre-init params were found, could not be set."));
            return;
        }

        if(!m_scopeModel->deInit()) {
            qWarning("Could not deinitialize Oscilloscope to set pre-init params.");
            setState(TState::TError, tr("Could not deinitialize Oscilloscope to set pre-init params."));
            return;
        }

        TConfigParam params = m_scopeModel->setPreInitParams(*preInitParams);

        if(!m_scopeModel->init()) {
            qWarning("Could not initialize Oscilloscope after setting pre-init params.");
            setState(TState::TError, tr("Could not initialize Oscilloscope after setting pre-init params."));
            return;
        }

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set pre-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString("[%1] Pre-init params were set").arg(m_scopeModel->name()));
    }

    void setPostInitParams() {
        TConfigParam * postInitParams = m_params.getSubParamByName("Oscilloscope configuration")->getSubParamByName("post-init params");

        if(!postInitParams) {
            qWarning("No post-init params were found, could not be set.");
            setState(TState::TError, tr("No post-init params were found, could not be set."));
            return;
        }

        TConfigParam params = m_scopeModel->setPostInitParams(*postInitParams);

        if(params.getState(true) == TConfigParam::TState::TError) {
            QString errorMessage = QString("Failed to set post-init parameters: %1").arg(params.getStateMessage());
            qWarning(errorMessage.toStdString().c_str());
            setState(TState::TError, errorMessage);
            return;
        }

        log(QString(tr("[%1] Post-init params were set")).arg(m_scopeModel->name()));
    }

    TScopeModel * getScopeModel() {
        TConfigParam * componentParam = m_params.getSubParamByName("Component");

        int componentCount = m_projectModel->componentContainer()->count();
        int selectedComponentIndex = componentCount > 0 ? 0 : -1;
        for(int i = 0; i < componentCount; i++) {
            QString componentName = m_projectModel->componentContainer()->at(i)->name();
            if(componentName == componentParam->getValue()) {
                selectedComponentIndex = i;
            }
        }

        TConfigParam * scopeParam = m_params.getSubParamByName("Oscilloscope");

        int selectedScopeIndex = -1;
        if(selectedComponentIndex > -1) {
            TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);

            int scopeCount = componentModel->scopeCount();
            selectedScopeIndex = scopeCount > 0 ? 0 : -1;
            for(int i = 0; i < scopeCount; i++) {
                QString scopeName = componentModel->scopeContainer()->at(i)->name();
                if(scopeName == scopeParam->getValue()) {
                    selectedScopeIndex = i;
                }
            }
        }

        if(selectedScopeIndex > -1) {
            TComponentModel * componentModel = m_projectModel->componentContainer()->at(selectedComponentIndex);
            TScopeModel * scopeModel = componentModel->scopeContainer()->at(selectedScopeIndex);

            return scopeModel;
        }

        return nullptr;
    }

    void execute(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
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

        connect(m_scopeModel, &TScopeModel::runFailed, this, &TScenarioScopeItem::runFailed);
        connect(m_scopeModel, &TScopeModel::stopFailed, this, &TScenarioScopeItem::stopFailed);
        connect(m_scopeModel, &TScopeModel::downloadFailed, this, &TScenarioScopeItem::downloadFailed);

        connect(m_scopeModel, &TScopeModel::tracesDownloaded, this, &TScenarioScopeItem::tracesDownloaded);
        connect(m_scopeModel, &TScopeModel::tracesEmpty, this, &TScenarioScopeItem::tracesEmpty);
        connect(m_scopeModel, &TScopeModel::stopped, this, &TScenarioScopeItem::stopped);
    }

    void tracesDownloaded(size_t traces, size_t samples, TScope::TSampleType type, QList<quint8 *> buffers, bool overvoltage) {
        m_outputData.clear();
        m_outputData.insert(getItemPortByName("traceCount"), QByteArray::number(traces));
        m_outputData.insert(getItemPortByName("sampleCount"), QByteArray::number(samples));

        int sampleSize;
        QString typeName;
        switch (type) {
            case TScope::TSampleType::TUInt8:   sampleSize = 1; typeName = "UInt8";     break;
            case TScope::TSampleType::TInt8:    sampleSize = 1; typeName = "Int8";      break;
            case TScope::TSampleType::TUInt16:  sampleSize = 2; typeName = "UInt16";    break;
            case TScope::TSampleType::TInt16:   sampleSize = 2; typeName = "Int16";     break;
            case TScope::TSampleType::TUInt32:  sampleSize = 4; typeName = "UInt32";    break;
            case TScope::TSampleType::TInt32:   sampleSize = 4; typeName = "Int32";     break;
            case TScope::TSampleType::TReal32:  sampleSize = 4; typeName = "Real32";    break;
            case TScope::TSampleType::TReal64:  sampleSize = 8; typeName = "Real64";    break;
            default: break;
        }
        m_outputData.insert(getItemPortByName("type"), typeName.toUtf8());

        QByteArray buffersData;
        for(int i = 0; i < traces; i++) {
            buffersData.append((const char *)buffers[i], samples*sampleSize);
        }
        m_outputData.insert(getItemPortByName("buffers"), buffersData);

        m_outputData.insert(getItemPortByName("overvoltage"), QByteArray::number(overvoltage ? 1 : 0));
    }

    void runFailed() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - run failed")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
        emit executionFinished();
    }

    void stopFailed() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - stop failed")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
        emit executionFinished();
    }

    void downloadFailed() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - download failed")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName  = "flowOutError";
        emit executionFinished();
    }

    void tracesEmpty() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - traces empty")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
        emit executionFinished();
    }

    void stopped() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Trace data downloaded")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOut";
        emit executionFinishedWithOutput(m_outputData);
    }


protected:
    QHash<TScenarioItemPort *, QByteArray> m_outputData;

    TScopeModel * m_scopeModel;
    bool m_isFirstBlockExecution;
};

#endif // TSCENARIOSCOPEITEM_H
