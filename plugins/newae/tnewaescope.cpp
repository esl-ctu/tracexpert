
#include "tnewaescope.h"

TnewaeScope::TnewaeScope(const QString & name_in, const QString & info_in, uint8_t id_in, TNewae * plugin_in, bool createdManually_in) { //sn i id musí být přes tconfigparam
    m_createdManually = createdManually_in;
    m_preInitParams = TConfigParam("NewAE " + name_in + " config", "", TConfigParam::TType::TDummy, "");
    if (!m_createdManually){
        m_preInitParams.addSubParam(TConfigParam("Serial number", info_in, TConfigParam::TType::TString,
                                                 "Serial number of the NewAE device. RO for autodetected devices.",
                                                 true));
    } else {
        m_preInitParams.addSubParam(TConfigParam("Serial number", QString(""), TConfigParam::TType::TString,
                                                 "Serial number of the NewAE device. RO for autodetected devices.",
                                                 false));
    }
    cwId = id_in;
    name = name_in;
    m_initialized = false;
    plugin = plugin_in;
    info = info_in;
    traceWaitingForRead = false;
}

TConfigParam TnewaeScope::_createPostInitParams(){
    auto top = TConfigParam("NewAE scope sn: " + sn + " config", "", TConfigParam::TType::TDummy, "");

    auto gain = TConfigParam("Gain", "", TConfigParam::TType::TDummy, "GainSettings");
    auto adc = TConfigParam("ADC", "", TConfigParam::TType::TDummy, "TriggerSettings");
    auto clock = TConfigParam("Clock", "", TConfigParam::TType::TDummy, "ClockSettings");
    auto io = TConfigParam("IO", "", TConfigParam::TType::TDummy, "GPIOSettings");
    auto trigger = TConfigParam("Trigger", "", TConfigParam::TType::TDummy, "TriggerSettings");
    auto glitch = TConfigParam("Glitch", "", TConfigParam::TType::TDummy, "GlitchSettings");

    //Gain
    gain.addSubParam(TConfigParam("db", QString(""), TConfigParam::TType::TReal, ""));
    gain.addSubParam(TConfigParam("gain", QString(""), TConfigParam::TType::TInt, ""));
    gain.addSubParam(TConfigParam("mode", QString(""), TConfigParam::TType::TString, ""));

    //ADC
    adc.addSubParam(TConfigParam("basic_mode", QString(""), TConfigParam::TType::TString, ""));
    adc.addSubParam(TConfigParam("clip_errors_disabled", QString(""), TConfigParam::TType::TBool, ""));
    adc.addSubParam(TConfigParam("decimate", QString(""), TConfigParam::TType::TInt, ""));
    adc.addSubParam(TConfigParam("lo_gain_errors_disabled", QString(""), TConfigParam::TType::TBool, ""));
    adc.addSubParam(TConfigParam("offset", QString(""), TConfigParam::TType::TUInt, ""));
    adc.addSubParam(TConfigParam("presamples", QString(""), TConfigParam::TType::TInt, ""));
    adc.addSubParam(TConfigParam("samples", QString(""), TConfigParam::TType::TInt, ""));
    adc.addSubParam(TConfigParam("state", QString(""), TConfigParam::TType::TBool, "", true));
    adc.addSubParam(TConfigParam("timeout", QString(""), TConfigParam::TType::TReal, ""));
    adc.addSubParam(TConfigParam("trig_count", QString(""), TConfigParam::TType::TInt, ""));

    //Clock
    clock.addSubParam(TConfigParam("adc_freq", QString(""), TConfigParam::TType::TInt, ""));
    clock.addSubParam(TConfigParam("adc_locked", QString(""), TConfigParam::TType::TBool, "", true));
    clock.addSubParam(TConfigParam("adc_phase", QString(""), TConfigParam::TType::TInt, ""));
    clock.addSubParam(TConfigParam("adc_rate", QString(""), TConfigParam::TType::TReal, "", true));
    clock.addSubParam(TConfigParam("adc_src", QString(""), TConfigParam::TType::TString, ""));
    clock.addSubParam(TConfigParam("clkgen_div", QString(""), TConfigParam::TType::TInt, ""));
    clock.addSubParam(TConfigParam("clkgen_freq", QString(""), TConfigParam::TType::TReal, ""));
    clock.addSubParam(TConfigParam("clkgen_locked", QString(""), TConfigParam::TType::TBool, "", true));
    clock.addSubParam(TConfigParam("clkgen_mul", QString(""), TConfigParam::TType::TInt, ""));
    clock.addSubParam(TConfigParam("clkgen_src", QString(""), TConfigParam::TType::TString, ""));
    clock.addSubParam(TConfigParam("enabled", QString(""), TConfigParam::TType::TBool, ""));
    clock.addSubParam(TConfigParam("extclk_freq", QString(""), TConfigParam::TType::TInt, ""));
    clock.addSubParam(TConfigParam("freq_ctr", QString(""), TConfigParam::TType::TInt, "", true));
    clock.addSubParam(TConfigParam("freq_ctr_src", QString(""), TConfigParam::TType::TString, ""));

    //IO
    io.addSubParam(TConfigParam("cdc_settings", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("extclk_src", QString(""), TConfigParam::TType::TString, "", true));
    io.addSubParam(TConfigParam("glitch_hp", QString(""), TConfigParam::TType::TBool, ""));
    io.addSubParam(TConfigParam("glitch_lp", QString(""), TConfigParam::TType::TBool, ""));
    io.addSubParam(TConfigParam("hs2", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("nrst", QString(""), TConfigParam::TType::TString, "", true));
    io.addSubParam(TConfigParam("pdic", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("pdid", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("target_pwr", QString(""), TConfigParam::TType::TBool, ""));
    io.addSubParam(TConfigParam("tio1", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("tio2", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("tio3", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("tio4", QString(""), TConfigParam::TType::TString, ""));
    io.addSubParam(TConfigParam("tio_states", QString(""), TConfigParam::TType::TBool, ""));
    io.addSubParam(TConfigParam("vcc_glitcht", QString(""), TConfigParam::TType::TInt, "", true));

    //Trigger
    trigger.addSubParam(TConfigParam("triggers", QString(""), TConfigParam::TType::TString, ""));
    trigger.addSubParam(TConfigParam("module", QString(""), TConfigParam::TType::TString, "", true));
    //Add more for CW Pro?

    //Glitch
    glitch.addSubParam(TConfigParam("arm_timing", QString(""), TConfigParam::TType::TString, ""));
    glitch.addSubParam(TConfigParam("clk_src", QString(""), TConfigParam::TType::TString, ""));
    glitch.addSubParam(TConfigParam("ext_offset", QString(""), TConfigParam::TType::TInt, ""));
    glitch.addSubParam(TConfigParam("offset", QString(""), TConfigParam::TType::TInt, ""));
    glitch.addSubParam(TConfigParam("offset_fine", QString(""), TConfigParam::TType::TInt, ""));
    glitch.addSubParam(TConfigParam("output", QString(""), TConfigParam::TType::TInt, ""));
    glitch.addSubParam(TConfigParam("repeat", QString(""), TConfigParam::TType::TString, ""));
    glitch.addSubParam(TConfigParam("trigger_src", QString(""), TConfigParam::TType::TString, ""));
    glitch.addSubParam(TConfigParam("width", QString(""), TConfigParam::TType::TReal, ""));
    glitch.addSubParam(TConfigParam("width_fine", QString(""), TConfigParam::TType::TInt, ""));

    //TODO funkce


    top.addSubParam(gain);
    top.addSubParam(adc);
    top.addSubParam(clock);
    top.addSubParam(io);
    top.addSubParam(trigger);
    top.addSubParam(glitch);

    return top;
}

uint8_t TnewaeScope::getId(){
    return cwId;
}

QList<TScope::TChannelStatus> TnewaeScope::getChannelsStatus(){
    //TODO
}

void TnewaeScope::notConnectedError() {
    qWarning("%s", (QString("NewAE device with serial number ") + QString(sn) + QString(" was disconnected. Please de-init and re-init the scope and device.")).toLocal8Bit().constData());
}

bool TnewaeScope::isInitialized(){
    return m_initialized;
}

bool TnewaeScope::_validatePreInitParamsStructure(TConfigParam & params){
    if (m_createdManually){
        bool iok;
        auto tmp = params.getSubParamByName("Serial number", &iok);
        if(!iok) {
            qDebug("Parameter does not exist");
            return false;
        }

        if (tmp->getValue().size() <= 0){
            qDebug("Wrong structure of the pre-init params for NewAE scope.");
            params.setState(TConfigParam::TState::TError, "Wrong structure of the pre-init params for NewAE scope.");
            return false;
        }

        //If the scope was created manually, we need to check if there are any duplicities
        //If the device is added manually, all other uninitalized devices need to be initialize first.
        if(m_createdManually) {
            bool duplicate = false;

            QList<TScope *> scopeList = plugin->getScopes();
            int notInitializedScopesCounter = 0;

            for (int i = 0; i < scopeList.length(); ++i){
                TnewaeScope * sc = (TnewaeScope *) scopeList.at(i);
                if (!(sc->isInitialized())) {
                    notInitializedScopesCounter++;
                }
            }

            if (notInitializedScopesCounter != 1) {
                qWarning("All other uninitalized devices need to be initialized first. Please initialize all autodetected devices and add only one manual device at a time.");
                return false;
            }

            sn = tmp->getValue();

            for (int i = 0; i < scopeList.length(); ++i){
                TnewaeScope * sc = (TnewaeScope *) scopeList.at(i);
                QString scSn = sc->getScopeSn();
                if (scSn == sn) {
                    duplicate = true;
                }
            }

            if (duplicate) {
                qWarning("A device with the same serial number already exists.");
                params.setState(TConfigParam::TState::TError, "A device with the same serial number already exists.");
                return false;
            }
        }
    }

    return true;
}

bool TnewaeScope::_validatePostInitParamsStructure(TConfigParam & params){
    //TODO
}

TnewaeScope::~TnewaeScope() {
    TnewaeScope::deInit();
}

QString TnewaeScope::getName() const{
    return m_name;
}

QString TnewaeScope::getInfo() const{
    return m_info;
}

QString TnewaeScope::getScopeSn() const{
    return sn;
}

TConfigParam TnewaeScope::getPreInitParams() const{
    return m_preInitParams;
}

TConfigParam TnewaeScope::setPreInitParams(TConfigParam params){
    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

void TnewaeScope::init(bool *ok/* = nullptr*/){
    bool succ = _validatePreInitParamsStructure(m_preInitParams);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }


    auto tmpSn = m_preInitParams.getSubParamByName("Serial number", &succ);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }
    sn = tmpSn->getValue();

    QString toSend;
    QList<QString> params;
    params.append(sn);
    plugin->packageDataForPython(cwId, "SETUP", 1, params, toSend);
    succ = plugin->writeToPython(cwId, toSend);
    succ &= plugin->waitForPythonDone(cwId, true);

    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    m_postInitParams = _createPostInitParams();
    m_postInitParams = updatePostInitParams(m_postInitParams);

    if(ok != nullptr) *ok = true;
    m_initialized = true;
}

void TnewaeScope::deInit(bool *ok/* = nullptr*/){
    m_initialized = false;

    QString toSend;
    QList<QString> params;
    plugin->packageDataForPython(cwId, "DEINI", 0, params, toSend);
    bool succ = plugin->writeToPython(cwId, toSend);
    succ &= plugin->waitForPythonDone(cwId, true);

    if(ok != nullptr) *ok = succ;

}

TConfigParam TnewaeScope::updatePostInitParams(TConfigParam paramsIn, bool write /*= false*/) const {
    TConfigParam topPrm = paramsIn;
    QList<TConfigParam> prms = topPrm.getSubParams();

    for(int i = 0; i < prms.length(); ++i){
        QString prmName = prms[i].getName();
        QList<TConfigParam> subPrms = prms[i].getSubParams();

        if (subPrms.length() == 0){
            QString out;
            if (write) {
                bool ok, ok2, ok3;
                ok = plugin->setPythonParameter(cwId, prmName, paramsIn.getSubParamByName(prmName)->getValue(), out);
                paramsIn.getSubParamByName(prmName, &ok2)->setValue(out, &ok3);
                if (!(ok & ok2 & ok3)) paramsIn.setState(TConfigParam::TState::TError, "Cannot read some params.");
            } else {
                bool ok, ok2, ok3;
                ok = plugin->getPythonParameter(cwId, prmName, out);
                paramsIn.getSubParamByName(prmName, &ok2)->setValue(out, &ok3);
                if (!(ok & ok2 & ok3)) paramsIn.setState(TConfigParam::TState::TError, "Cannot read some params.");
            }
        }

        if (!(subPrms.length() == 1 && subPrms[0].getName() == "Call function?")){ //make sure that this is not only a subparam for a function call
            for (int j = 0; j < subPrms.length(); ++j){
                QList<TConfigParam> subSubPrms = subPrms[i].getSubParams();

                QString subPrmName = subPrms[i].getName();
                if (subSubPrms.length() == 0){
                    QString out;
                    if (write) {
                        bool ok, ok2, ok3, ok4;
                        ok = plugin->setPythonSubparameter(cwId, prmName, subPrmName, paramsIn.getSubParamByName(prmName)->getSubParamByName(subPrmName)->getValue(), out);
                        paramsIn.getSubParamByName(prmName, &ok2)->getSubParamByName(subPrmName, &ok3)->setValue(out, &ok4);
                        if (!(ok & ok2 & ok3 & ok4)) paramsIn.setState(TConfigParam::TState::TError, "Cannot read some params.");
                    } else {
                        bool ok, ok2, ok3, ok4;
                        ok = plugin->getPythonSubparameter(cwId, prmName, subPrmName, out);
                        paramsIn.getSubParamByName(prmName, &ok2)->getSubParamByName(subPrmName, &ok3)->setValue(out, &ok4);
                        if (!(ok & ok2 & ok3 & ok4)) paramsIn.setState(TConfigParam::TState::TError, "Cannot read some params.");
                    }
                } else {
                    if (subSubPrms[0].getValue() == "Yes" && write){
                        size_t len;
                        QString out;
                        bool ok;
                        //TODO
                        ok = plugin->runPythonFunctionOnAnObjectAndGetStringOutput(cwId, prmName, subPrmName, len, out);
                        subSubPrms[0].setValue("No");
                        if (!ok) paramsIn.setState(TConfigParam::TState::TError, "Cannot read/write some params.");
                    }
                }
            }
        } else { //Call function
            if (subPrms[0].getValue() == "Yes" && write){
                QList<QString> tmp;
                size_t len;
                QString out;
                bool ok, ok2;
                ok = plugin->runPythonFunctionAndGetStringOutput(cwId, prmName, 0, tmp, len, out);
                subPrms[0].setValue("No", &ok2);
                if (!(ok & ok2)) paramsIn.setState(TConfigParam::TState::TError, "Cannot read/write some params.");
            }
        }
    }

    return paramsIn;
}

TConfigParam TnewaeScope::getPostInitParams() const{
    TConfigParam params = m_postInitParams;
    return updatePostInitParams(params);
}

TConfigParam TnewaeScope::setPostInitParams(TConfigParam params){  
    m_postInitParams = updatePostInitParams(params, true);
    return m_postInitParams;
}

void TnewaeScope::run(size_t * expectedBufferSize, bool *ok){
    //Comment: This function could probably use capture_segmented from the CW docs. However, the timeout is not handled on the CW side yet
    //         It would be a good idea to use that function once newae fixes that
    bool succ;
    QList<QString> params;
    size_t dataLen;
    QString response;

    params.clear();
    succ = plugin->runPythonFunctionAndGetStringOutput(cwId, "capture", 0, params, dataLen, response);

    if (!succ) {
        qDebug("Error sending the capture command. This does not necessarily mean a timeout.");
        if(ok != nullptr) *ok = false;
    }

    if (response != "False") {
        qDebug("%s", (QString("Capture timed out. CW reponse to timeot querry: ") + QString(response)).toLocal8Bit().constData());
        if(ok != nullptr) *ok = false;
    }

    traceWaitingForRead = true;
    *expectedBufferSize = cwBufferSize;

    if(ok != nullptr) *ok = true;

}
void TnewaeScope::stop(bool *ok){
    qDebug("The run method for newae is blocking. This stop() method does not do anything. The capture cannot be running when calling this.");
    if(ok != nullptr) *ok = true;
}

size_t TnewaeScope::downloadSamples(int channel, uint8_t * buffer, size_t bufferSize,
                                    TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage){
    *samplesType = TSampleType::TReal64;
    *samplesPerTraceDownloaded = 0;
    *tracesDownloaded = 0;

    if (channel != 0) {
        qWarning("Wriong channel!");
        return 0;
    }

    //The following is future code for downloading multiple traces - it relies on getTracesFromShm() which is done
    //and on support in python which is very much not done
    /*QList<double> traces;
    bool ok = plugin->getTracesFromShm(tracesDownloaded, samplesPerTraceDownloaded, traces);
    if (!ok) {
        return 0;
    }

    size_t internalSize = bufferSize / 8;
    double * internalBuffer = (double *) buffer;
    size_t i = 0;
    for (; (i < internalSize) && (i < tracesDownloaded * samplesPerTraceDownloaded); ++i){
        internalBuffer[i] = traces.at(i);
    }*/

    if (!traceWaitingForRead) {
        //run();
    }

    if (!traceWaitingForRead) {
        return 0;
    }

    bool succ;
    QList<QString> params;
    size_t dataLen;
    QString response;

    params.clear();
    params.append("false"); //Get traces as doubles
    succ = plugin->runPythonFunctionAndGetStringOutput(cwId, "get_last_trace", params.count(), params, dataLen, response);

    if (!succ) {
        qDebug("Error sending the get_last_trace command.");
        return 0;
    }

    traceWaitingForRead = false;

    *tracesDownloaded = 1;
    *samplesPerTraceDownloaded = dataLen/8;
    *overvoltage = false; //TODO

    size_t maxSize = dataLen > bufferSize ? bufferSize : dataLen;

    memcpy(buffer, response.toLocal8Bit().constData(), maxSize);

    return maxSize;
}
