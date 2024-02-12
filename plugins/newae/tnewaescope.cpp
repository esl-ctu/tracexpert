
#include "tnewaescope.h"

bool validateParamD(QString in, double min, double max){
    bool ok;
    double val = in.toDouble(&ok);
    if (!ok) return false;

    if (val >= min && val <= max) return true;
    return false;
}

bool validateParamLL(QString in, long long min, long long max){
    bool ok;
    int val = in.toInt(&ok);
    if (!ok) return false;

    if (val >= min && val <= max) return true;
    return false;
}

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

    m_preInitParams.addSubParam(TConfigParam("Memory depth of the oscilloscope", QString("24400"), TConfigParam::TType::TUInt,
                                             "Memory depth of the oscilloscope. Edit only if you are not using CW lite and you know what you are doing.",
                                             false));

    cwId = id_in;
    m_name = name_in;
    name = name_in;
    m_initialized = false;
    plugin = plugin_in;
    info = info_in;
    m_info = info_in;
    traceWaitingForRead = false;

    stopNow = false;
    running = false;

    chanStatus.append(TChannelStatus(0, "Chipwhisperer ch0", true, 0.5, 0)); //TODO: je to dobře?
}

TConfigParam TnewaeScope::_createPostInitParams(){
    auto top = TConfigParam("NewAE scope sn: " + sn + " config", "", TConfigParam::TType::TDummy, "");

    auto NewAE = TConfigParam("NewAE", "", TConfigParam::TType::TDummy, "Parameters for the NewAE device");
    auto TraceXpert = TConfigParam("TraceXpert", "", TConfigParam::TType::TDummy, "Parameters for this SW");

    auto gain = TConfigParam("Gain", "", TConfigParam::TType::TDummy, "GainSettings");
    auto adc = TConfigParam("ADC", "", TConfigParam::TType::TDummy, "TriggerSettings");
    auto clock = TConfigParam("Clock", "", TConfigParam::TType::TDummy, "ClockSettings");
    auto io = TConfigParam("IO", "", TConfigParam::TType::TDummy, "GPIOSettings");
    auto trigger = TConfigParam("Trigger", "", TConfigParam::TType::TDummy, "TriggerSettings");
    auto glitch = TConfigParam("Glitch", "", TConfigParam::TType::TDummy, "GlitchSettings");

    //Top
    auto fun1 = TConfigParam("default_setup", QString(""), TConfigParam::TType::TDummy, "");
    fun1.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    top.addSubParam(fun1);
    auto fun2 = TConfigParam("cglitch_setup", QString(""), TConfigParam::TType::TDummy, "");
    fun2.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    top.addSubParam(fun2);
    auto fun3 = TConfigParam("vglitch_setup", QString(""), TConfigParam::TType::TDummy, "");
    fun3.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    top.addSubParam(fun3);
    auto fun4 = TConfigParam("reset_sam3u", QString(""), TConfigParam::TType::TDummy, "");
    fun4.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, "You cannot call this function here. It would disconnect the scope!", true));
    top.addSubParam(fun4);
    top.addSubParam(TConfigParam("fw_version_str", QString(""), TConfigParam::TType::TString, "", true));
    top.addSubParam(TConfigParam("sn", QString(""), TConfigParam::TType::TString, "", true));


    //Gain
    gain.addSubParam(TConfigParam("db", QString(""), TConfigParam::TType::TReal, ""));
    gain.addSubParam(TConfigParam("gain", QString(""), TConfigParam::TType::TInt, ""));
    auto gEnum1 = TConfigParam("mode", QString(""), TConfigParam::TType::TEnum, "");
    gEnum1.addEnumValue("low");
    gEnum1.addEnumValue("high");
    gain.addSubParam(gEnum1);

    //ADC
    auto aEnum1 = TConfigParam("basic_mode", QString(""), TConfigParam::TType::TEnum, "");
    aEnum1.addEnumValue("low");
    aEnum1.addEnumValue("high");
    aEnum1.addEnumValue("rising_edge");
    aEnum1.addEnumValue("falling_edge");
    adc.addSubParam(aEnum1);
    //adc.addSubParam(TConfigParam("clip_errors_disabled", QString(""), TConfigParam::TType::TBool, "")); (husky only)
    adc.addSubParam(TConfigParam("decimate", QString(""), TConfigParam::TType::TUInt, ""));
    //adc.addSubParam(TConfigParam("lo_gain_errors_disabled", QString(""), TConfigParam::TType::TBool, "")); (husky only)
    adc.addSubParam(TConfigParam("offset", QString(""), TConfigParam::TType::TUInt, ""));
    adc.addSubParam(TConfigParam("presamples", QString(""), TConfigParam::TType::TInt, ""));
    adc.addSubParam(TConfigParam("samples", QString(""), TConfigParam::TType::TInt, ""));
    adc.addSubParam(TConfigParam("state", QString(""), TConfigParam::TType::TBool, "", true));
    adc.addSubParam(TConfigParam("timeout", QString(""), TConfigParam::TType::TReal, ""));
    adc.addSubParam(TConfigParam("trig_count", QString(""), TConfigParam::TType::TInt, "", true));
    //auto funA1 = TConfigParam("clear_clip_errors", QString(""), TConfigParam::TType::TDummy, ""); (husky only)
    //funA1.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    //adc.addSubParam(funA1);

    //Clock
    clock.addSubParam(TConfigParam("adc_freq", QString(""), TConfigParam::TType::TInt, "", true));
    clock.addSubParam(TConfigParam("adc_locked", QString(""), TConfigParam::TType::TBool, "", true));
    clock.addSubParam(TConfigParam("adc_phase", QString(""), TConfigParam::TType::TInt, ""));
    clock.addSubParam(TConfigParam("adc_rate", QString(""), TConfigParam::TType::TReal, "", true));
    auto cEnum1 = TConfigParam("adc_src", QString(""), TConfigParam::TType::TEnum, "");
    cEnum1.addEnumValue("clkgen_x1");
    cEnum1.addEnumValue("clkgen_x4");
    cEnum1.addEnumValue("extclk_x1");
    cEnum1.addEnumValue("extclk_x4");
    cEnum1.addEnumValue("extclk_dir");
    clock.addSubParam(cEnum1);
    clock.addSubParam(TConfigParam("clkgen_div", QString(""), TConfigParam::TType::TInt, ""));
    clock.addSubParam(TConfigParam("clkgen_freq", QString(""), TConfigParam::TType::TReal, ""));
    clock.addSubParam(TConfigParam("clkgen_locked", QString(""), TConfigParam::TType::TBool, "", true));
    clock.addSubParam(TConfigParam("clkgen_mul", QString(""), TConfigParam::TType::TInt, ""));
    auto cEnum2 = TConfigParam("clkgen_src", QString(""), TConfigParam::TType::TEnum, "");
    cEnum2.addEnumValue("extclk");
    cEnum2.addEnumValue("system");
    cEnum2.addEnumValue("internal");
    clock.addSubParam(cEnum2);
    //clock.addSubParam(TConfigParam("enabled", QString(""), TConfigParam::TType::TBool, ""));
    clock.addSubParam(TConfigParam("extclk_freq", QString(""), TConfigParam::TType::TUInt, ""));
    clock.addSubParam(TConfigParam("freq_ctr", QString(""), TConfigParam::TType::TInt, "", true));
    auto cEnum3 = TConfigParam("freq_ctr_src", QString(""), TConfigParam::TType::TEnum, "");
    cEnum3.addEnumValue("clkgen");
    cEnum3.addEnumValue("extclk");
    clock.addSubParam(cEnum3);
    auto funC1 = TConfigParam("reset_adc", QString(""), TConfigParam::TType::TDummy, "");
    funC1.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    clock.addSubParam(funC1);
    auto funC2 = TConfigParam("reset_clkgen", QString(""), TConfigParam::TType::TDummy, "");
    funC2.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    clock.addSubParam(funC2);
    auto funC3 = TConfigParam("reset_dcms", QString(""), TConfigParam::TType::TDummy, "");
    funC3.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    clock.addSubParam(funC3);

    //IO
    io.addSubParam(TConfigParam("cdc_settings", QString(""), TConfigParam::TType::TUInt, ""));
    io.addSubParam(TConfigParam("extclk_src", QString(""), TConfigParam::TType::TString, "", true));
    io.addSubParam(TConfigParam("glitch_hp", QString(""), TConfigParam::TType::TBool, ""));
    io.addSubParam(TConfigParam("glitch_lp", QString(""), TConfigParam::TType::TBool, ""));
    auto iEnum1 = TConfigParam("hs2", QString(""), TConfigParam::TType::TEnum, "");
    iEnum1.addEnumValue("clkgen");
    iEnum1.addEnumValue("glitch");
    iEnum1.addEnumValue("disabled");
    io.addSubParam(iEnum1);
    auto iEnum2 = TConfigParam("nrst", QString(""), TConfigParam::TType::TEnum, "");
    iEnum2.addEnumValue("high");
    iEnum2.addEnumValue("low");
    iEnum2.addEnumValue("high_z");
    io.addSubParam(iEnum2);
    auto iEnum3 = TConfigParam("pdic", QString(""), TConfigParam::TType::TEnum, "");
    iEnum3.addEnumValue("high");
    iEnum3.addEnumValue("low");
    iEnum3.addEnumValue("high_z");
    io.addSubParam(iEnum3);
    auto iEnum4 = TConfigParam("pdid", QString(""), TConfigParam::TType::TEnum, "");
    iEnum4.addEnumValue("high");
    iEnum4.addEnumValue("low");
    iEnum4.addEnumValue("high_z");
    io.addSubParam(iEnum4);
    io.addSubParam(TConfigParam("target_pwr", QString(""), TConfigParam::TType::TBool, ""));
    auto iEnum5 = TConfigParam("tio1", QString(""), TConfigParam::TType::TEnum, "");
    iEnum5.addEnumValue("serial_rx");
    iEnum5.addEnumValue("serial_tx");
    iEnum5.addEnumValue("high_z");
    iEnum5.addEnumValue("gpio_low");
    iEnum5.addEnumValue("gpio_high");
    iEnum5.addEnumValue("gpio_disabled");
    io.addSubParam(iEnum5);
    auto iEnum6 = TConfigParam("tio2", QString(""), TConfigParam::TType::TEnum, "");
    iEnum6.addEnumValue("serial_rx");
    iEnum6.addEnumValue("serial_tx");
    iEnum6.addEnumValue("high_z");
    iEnum6.addEnumValue("gpio_low");
    iEnum6.addEnumValue("gpio_high");
    iEnum6.addEnumValue("gpio_disabled");
    io.addSubParam(iEnum6);
    auto iEnum7 = TConfigParam("tio3", QString(""), TConfigParam::TType::TEnum, "");
    iEnum7.addEnumValue("serial_rx");
    iEnum7.addEnumValue("serial_tx");
    iEnum7.addEnumValue("serial_tx_rx");
    iEnum7.addEnumValue("high_z");
    iEnum7.addEnumValue("gpio_low");
    iEnum7.addEnumValue("gpio_high");
    iEnum7.addEnumValue("gpio_disabled");
    io.addSubParam(iEnum7);
    auto iEnum8 = TConfigParam("tio4", QString(""), TConfigParam::TType::TEnum, "");
    iEnum8.addEnumValue("serial_tx");
    iEnum8.addEnumValue("high_z");
    iEnum8.addEnumValue("gpio_low");
    iEnum8.addEnumValue("gpio_high");
    iEnum8.addEnumValue("gpio_disabled");
    io.addSubParam(iEnum8);
    //io.addSubParam(TConfigParam("tio_states", QString(""), TConfigParam::TType::TBool, ""));
    //io.addSubParam(TConfigParam("vcc_glitcht", QString(""), TConfigParam::TType::TInt, "", true));
    auto funI1 = TConfigParam("vglitch_disable", QString(""), TConfigParam::TType::TDummy, "");
    funI1.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    io.addSubParam(funI1);
    auto funI2 = TConfigParam("vglitch_reset", QString(""), TConfigParam::TType::TDummy, "");
    funI2.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    io.addSubParam(funI2);

    //Trigger
    trigger.addSubParam(TConfigParam("triggers", QString(""), TConfigParam::TType::TString, "Refer to CW docs. This parameter CANNOT be verified inside TraceXpert! The whole \
                                                                                            config param struct is gonna be marked as invalid if you write an invalid expression here."));
    trigger.addSubParam(TConfigParam("module", QString(""), TConfigParam::TType::TString, "", true));

    //Glitch - continue from here
    auto glEnum1 = TConfigParam("arm_timing", QString(""), TConfigParam::TType::TEnum, "");
    glEnum1.addEnumValue("no_glitch");
    glEnum1.addEnumValue("before_scope");
    glEnum1.addEnumValue("after_scope");
    glitch.addSubParam(glEnum1);
    auto glEnum2 = TConfigParam("clk_src", QString(""), TConfigParam::TType::TEnum, "");
    glEnum2.addEnumValue("target");
    glEnum2.addEnumValue("clkgen");
    glEnum2.addEnumValue("pll");
    glitch.addSubParam(glEnum2);
    glitch.addSubParam(TConfigParam("ext_offset", QString(""), TConfigParam::TType::TInt, ""));
    glitch.addSubParam(TConfigParam("offset", QString(""), TConfigParam::TType::TReal, ""));
    //DO NOT change the hint string! It is used to recognize a write only parameter in the code later on!
    glitch.addSubParam(TConfigParam("offset_fine", QString("0"), TConfigParam::TType::TInt, "Write-only, reads return zero"));
    auto glEnum3 = TConfigParam("output", QString(""), TConfigParam::TType::TEnum, "");
    glEnum3.addEnumValue("clock_only");
    glEnum3.addEnumValue("glitch_only");
    glEnum3.addEnumValue("clock_or");
    glEnum3.addEnumValue("clock_xor");
    glEnum3.addEnumValue("enable_only");
    glitch.addSubParam(glEnum3);
    glitch.addSubParam(TConfigParam("repeat", QString(""), TConfigParam::TType::TInt, ""));
    auto glEnum4 = TConfigParam("trigger_src", QString(""), TConfigParam::TType::TEnum, "");
    glEnum4.addEnumValue("continuous");
    glEnum4.addEnumValue("manual");
    glEnum4.addEnumValue("ext_single");
    glEnum4.addEnumValue("ext_continuous");
    glitch.addSubParam(glEnum4);
    glitch.addSubParam(TConfigParam("width", QString(""), TConfigParam::TType::TReal, ""));
    //DO NOT change the hint string! It is used to recognize a write only parameter in the code later on!
    glitch.addSubParam(TConfigParam("width_fine", QString("0"), TConfigParam::TType::TInt, "Write-only, reads return zero"));
    auto funG1 = TConfigParam("manual_trigger", QString(""), TConfigParam::TType::TDummy, "");
    funG1.addSubParam(TConfigParam("Run?", QString("false"), TConfigParam::TType::TBool, ""));
    glitch.addSubParam(funG1);
    //read_status()??

    NewAE.addSubParam(gain);
    NewAE.addSubParam(adc);
    NewAE.addSubParam(clock);
    NewAE.addSubParam(io);
    NewAE.addSubParam(trigger);
    NewAE.addSubParam(glitch);

    TraceXpert.addSubParam(TConfigParam("Get traces as int", QString("true"), TConfigParam::TType::TBool, ""));
    auto tEnum1 = TConfigParam("Mode", QString("Triggered"), TConfigParam::TType::TEnum, "");
    tEnum1.addEnumValue("Triggered");
    tEnum1.addEnumValue("Continuous");
    TraceXpert.addSubParam(tEnum1);

    top.addSubParam(NewAE);
    top.addSubParam(TraceXpert);

    return top;
}

uint8_t TnewaeScope::getId(){
    return cwId;
}

QString TnewaeScope::getSn(){
    return sn;
}

QList<TScope::TChannelStatus> TnewaeScope::getChannelsStatus(){
    int index = 0;
    QString alias = name + " ch0";
    bool enabled = true;
    qreal range = 0.5;
    qreal offset = 0;

    QString out;
    bool ok = plugin->getPythonSubparameter(cwId, "ADC", "offset", out);
    offset = out.toDouble();

    TChannelStatus channelA(index, alias, enabled, range, offset);

    QList<TnewaeScope::TChannelStatus> channelList;
    channelList.append(channelA);
    return channelList;
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
    bool ok = true;
    TConfigParam * newAEparams = params.getSubParamByName("NewAE");

    if(!validateParamD(newAEparams->getSubParamByName("Gain")->getSubParamByName("db")->getValue(), -6.5, 56)){
        newAEparams->getSubParamByName("Gain")->getSubParamByName("db")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamLL(newAEparams->getSubParamByName("Gain")->getSubParamByName("gain")->getValue(), 0, 78)){
        newAEparams->getSubParamByName("Gain")->getSubParamByName("gain")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamLL(newAEparams->getSubParamByName("ADC")->getSubParamByName("presamples")->getValue(), 0, cwBufferSize)){
        newAEparams->getSubParamByName("ADC")->getSubParamByName("presamples")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamLL(newAEparams->getSubParamByName("ADC")->getSubParamByName("samples")->getValue(), 0, cwBufferSize)){
        newAEparams->getSubParamByName("ADC")->getSubParamByName("samples")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamD(newAEparams->getSubParamByName("ADC")->getSubParamByName("timeout")->getValue(), 0, INT32_MAX)){
        newAEparams->getSubParamByName("ADC")->getSubParamByName("timeout")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamLL(newAEparams->getSubParamByName("Clock")->getSubParamByName("adc_phase")->getValue(), -255, 255)){
        newAEparams->getSubParamByName("Clock")->getSubParamByName("adc_phase")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamLL(newAEparams->getSubParamByName("Clock")->getSubParamByName("clkgen_div")->getValue(), 1, 256)){
        newAEparams->getSubParamByName("Clock")->getSubParamByName("clkgen_div")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamD(newAEparams->getSubParamByName("Clock")->getSubParamByName("clkgen_freq")->getValue(), 3200000, INT32_MAX)){
        newAEparams->getSubParamByName("Clock")->getSubParamByName("clkgen_freq")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamLL(newAEparams->getSubParamByName("Clock")->getSubParamByName("clkgen_mul")->getValue(), 2, 256)){
        newAEparams->getSubParamByName("Clock")->getSubParamByName("clkgen_mul")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamLL(newAEparams->getSubParamByName("Glitch")->getSubParamByName("ext_offset")->getValue(), 0, INT32_MAX)){
        newAEparams->getSubParamByName("Glitch")->getSubParamByName("ext_offset")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamD(newAEparams->getSubParamByName("Glitch")->getSubParamByName("offset")->getValue(), -50, 50)){
        newAEparams->getSubParamByName("Glitch")->getSubParamByName("offset")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if (newAEparams->getSubParamByName("Glitch")->getSubParamByName("offset_fine")->getValue() != ""){
        if(!validateParamLL(newAEparams->getSubParamByName("Glitch")->getSubParamByName("offset_fine")->getValue(), -255, 255)){
            newAEparams->getSubParamByName("Glitch")->getSubParamByName("offset_fine")->setState(TConfigParam::TState::TError);
            ok = false;
        }
    }

    if (newAEparams->getSubParamByName("Glitch")->getSubParamByName("width_fine")->getValue() != ""){
        if(!validateParamLL(newAEparams->getSubParamByName("Glitch")->getSubParamByName("width_fine")->getValue(), -255, 255)){
            newAEparams->getSubParamByName("Glitch")->getSubParamByName("width_fine")->setState(TConfigParam::TState::TError);
            ok = false;
        }
    }

    if(!validateParamLL(newAEparams->getSubParamByName("Glitch")->getSubParamByName("repeat")->getValue(), 1, 8192)){
        newAEparams->getSubParamByName("Glitch")->getSubParamByName("repeat")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if(!validateParamD(newAEparams->getSubParamByName("Glitch")->getSubParamByName("width")->getValue(), -49.8, 49.8)){
        newAEparams->getSubParamByName("Glitch")->getSubParamByName("width")->setState(TConfigParam::TState::TError);
        ok = false;
    }

    if (!ok) qDebug("Validation of postinitparams did not pass!");

    return ok;
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

    QString cwBufferSizeStr = m_preInitParams.getSubParamByName("Memory depth of the oscilloscope", &succ)->getValue();
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }
    cwBufferSize = cwBufferSizeStr.toUInt(&succ);
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

//This whole method is ugly. I'm sorry
TConfigParam TnewaeScope::updatePostInitParams(TConfigParam paramsIn, bool write /*= false*/) const {
    bool ook;
    TConfigParam * topPrm = paramsIn.getSubParamByName("NewAE", &ook);
    if (!ook) {
        paramsIn.setState(TConfigParam::TState::TError, "Error getting scope params!");
        qWarning("Cannot find postinit params!");
        return paramsIn;
    }
    QList<TConfigParam> prms = topPrm->getSubParams();

    //state == 0 -> call any FUNCTIONs that shoulf be called
    //state == 1 -> set any PARAMs that should be set
    //This ensures that functions are called first
    for(int state = 0; state <= 1; state++) {

        for(int i = 0; i < prms.length(); ++i){
            QString prmName = prms[i].getName();
            QList<TConfigParam> subPrms = prms[i].getSubParams();

            if (state == 1) { //PARAMs
                if (subPrms.length() == 0){
                    QString out;
                    if (write && !(prms[i].isReadonly())) {
                        bool ok, ok2, ok3;
                        QString newVal = topPrm->getSubParamByName(prmName)->getValue();
                        QString oldVal = ((TConfigParam *) &m_postInitParams)->getSubParamByName("NewAE")->getSubParamByName(prmName)->getValue();
                        if (newVal != oldVal) {
                            ok = plugin->setPythonParameter(cwId, prmName, newVal, out);
                            topPrm->getSubParamByName(prmName, &ok2)->setValue(out.toLower(), &ok3);
                            if (!(ok & ok2 & ok3)) {
                                topPrm->setState(TConfigParam::TState::TError, "Cannot write params.");
                                qDebug("%s", ("Error writing param " + prmName).toLocal8Bit().constData());
                            }
                        }
                    } else {
                        bool ok, ok2, ok3;
                        bool isWriteOnly = topPrm->getSubParamByName(prmName, &ok)->getHint() == "Write-only, reads return zero";
                        if(!isWriteOnly && ok) { //If parameter is not write-only
                            ok = plugin->getPythonParameter(cwId, prmName, out);
                            topPrm->getSubParamByName(prmName, &ok2)->setValue(out.toLower(), &ok3);
                            if (!(ok & ok2 & ok3)) {
                                topPrm->setState(TConfigParam::TState::TWarning, "Cannot read some params.");
                                qDebug("%s", ("Error reading param " + prmName).toLocal8Bit().constData());
                            }
                        } else { //Do nothing - keep old value
                            //topPrm->getSubParamByName(prmName, &ok2)->setValue("0", &ok3);
                            //if (!(ok & ok2 & ok3)) topPrm->setState(TConfigParam::TState::TError, "Cannot read some params.");
                        }
                    }
                }
            }

            if (!(subPrms.length() == 1 && subPrms[0].getName() == "Run?")){ //make sure that this is not only a subparam for a function call
                for (int j = 0; j < subPrms.length(); ++j){
                    QList<TConfigParam> subSubPrms = subPrms[j].getSubParams();

                    QString subPrmName = subPrms[j].getName();
                    if (subSubPrms.length() == 0){
                        if (state == 1) { //PARAMs
                            QString out;
                            if (write && !(subPrms[j].isReadonly())) {
                                bool ok, ok2, ok3, ok4;
                                QString newVal = topPrm->getSubParamByName(prmName)->getSubParamByName(subPrmName)->getValue();
                                QString oldVal = ((TConfigParam *) &m_postInitParams)->getSubParamByName("NewAE")->getSubParamByName(prmName)->getSubParamByName(subPrmName)->getValue();
                                if (newVal != oldVal) {
                                    ok = plugin->setPythonSubparameter(cwId, prmName, subPrmName, newVal, out);
                                    topPrm->getSubParamByName(prmName, &ok2)->getSubParamByName(subPrmName, &ok3)->setValue(out.toLower(), &ok4);
                                    if (!(ok & ok2 & ok3 & ok4)){
                                        topPrm->setState(TConfigParam::TState::TError, "Cannot write some params.");
                                        qDebug("%s", ("Error writing subparam " + prmName + "->" + subPrmName).toLocal8Bit().constData());
                                        if (!ok)  qDebug("Error is in Python");
                                    }
                                }

                            } else {
                                bool ok, ok2, ok3, ok4;
                                bool isWriteOnly =topPrm->getSubParamByName(prmName, &ok2)->getSubParamByName(subPrmName, &ok3)->getHint() == "Write-only, reads return zero";
                                if(!isWriteOnly && ok2 && ok3) {
                                    ok = plugin->getPythonSubparameter(cwId, prmName, subPrmName, out);
                                    topPrm->getSubParamByName(prmName, &ok2)->getSubParamByName(subPrmName, &ok3)->setValue(out.toLower(), &ok4);
                                    if (!(ok & ok2 & ok3 & ok4)) {
                                        topPrm->setState(TConfigParam::TState::TWarning, "Cannot read some params.");
                                        qDebug("%s", ("Error reading subparam " + prmName + "->" + subPrmName).toLocal8Bit().constData());
                                    }
                                } else { //Do nothing - keep old value
                                    //topPrm->getSubParamByName(prmName, &ok2)->getSubParamByName(subPrmName, &ok3)->setValue("0", &ok4);
                                    //if (!(ok2 & ok3 & ok4)) topPrm->setState(TConfigParam::TState::TError, "Cannot read some params.");
                                }
                            }
                        }
                    } else { //Call function
                        if (state == 0) { //FUNCTIONs
                            if (subSubPrms[0].getValue() == "true" && write){
                                size_t len;
                                QString out;
                                bool ok;
                                ok = plugin->runPythonFunctionOnAnObjectAndGetStringOutput(cwId, prmName, subPrmName, len, out);
                                subSubPrms[0].setValue("No");
                                if (!ok) {
                                    topPrm->setState(TConfigParam::TState::TWarning, "Cannot read/write some params.");
                                    qDebug("%s", ("Error reading or writing (sub)param " + prmName).toLocal8Bit().constData());
                                }
                            }
                        }
                    }
                }
            } else { //Call function
                if (state == 0) { //FUNCTIONs
                    if (subPrms[0].getValue() == "true" && write){
                        QList<QString> tmp;
                        size_t len;
                        QString out;
                        bool ok, ok2;
                        ok = plugin->runPythonFunctionAndGetStringOutput(cwId, prmName, 0, tmp, len, out);
                        subPrms[0].setValue("No", &ok2);
                        if (!(ok & ok2)) {
                            topPrm->setState(TConfigParam::TState::TWarning, "Cannot read/write some params.");
                            qDebug("%s", ("Error reading or writing param " + prmName).toLocal8Bit().constData());
                        }
                    }
                }
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
    m_postInitParams.resetState(true);
    bool ok = _validatePostInitParamsStructure(params);
    if (ok) {
        m_postInitParams = updatePostInitParams(params, true);
        m_postInitParams = updatePostInitParams(params);
    }
    else {
        m_postInitParams = params;
    }
    return m_postInitParams;
}

void TnewaeScope::run(size_t * expectedBufferSize, bool *ok){
    //Comment: This function could probably use capture_segmented from the CW docs. However, the timeout is not handled on the CW side yet
    //         It would be a good idea to use that function once newae fixes that
    bool succ;
    QList<QString> params;
    size_t dataLen;
    QString response;

    running = true;

    params.clear();
    succ = plugin->runPythonFunctionAndGetStringOutput(cwId, "arm", 0, params, dataLen, response);
    if (!succ) {
        qDebug("Error sending the arm command. This does not necessarily mean a timeout.");
        if(ok != nullptr) *ok = false;
    }

    bool tracesAsInt = m_postInitParams.getSubParamByName("TraceXpert")->getSubParamByName("Get traces as int")->getValue() == "true";

    if (tracesAsInt){
        *expectedBufferSize = cwBufferSize*sizeof(uint16_t);
    } else {
        *expectedBufferSize = cwBufferSize*sizeof(double);
    }

    if(ok != nullptr) *ok = true;
    stopNow = false;

}
void TnewaeScope::stop(bool *ok){
    stopNow = true;
    running = false;
    if(ok != nullptr) *ok = true;
}

size_t TnewaeScope::downloadSamples(int channel, uint8_t * buffer, size_t bufferSize,
                                    TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage){
    *samplesPerTraceDownloaded = 0;
    *tracesDownloaded = 0;

    if (channel != 0) {
        qWarning("Wrong channel!");
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

    bool tracesAsInt = m_postInitParams.getSubParamByName("TraceXpert")->getSubParamByName("Get traces as int")->getValue() == "true";
    bool continuous = m_postInitParams.getSubParamByName("TraceXpert")->getSubParamByName("Mode")->getValue() == "Continuous";

    bool succ;
    QList<QString> params;
    size_t dataLen;
    QString response;

    params.clear();
    succ = plugin->runPythonFunctionAndGetStringOutput(cwId, "capture", 0, params, dataLen, response);

    if (!succ) {
        qDebug("Error sending the capture command. This does not necessarily mean a timeout.");
    }

    if (continuous){
        if (stopNow) {
            stopNow = false;
            running = false;
        } else {
            size_t throwaway;
            bool ook;
            run(&throwaway, &ook);
            if(!ook) {
                qWarning("Continuous mode not available right now. Switched to Triggered");
                m_postInitParams.getSubParamByName("TraceXpert")->getSubParamByName("Mode")->setValue("Triggered");
                m_postInitParams.getSubParamByName("TraceXpert")->getSubParamByName("Mode")->setState(TConfigParam::TState::TWarning, "Triggered mode unavailable");
                running = false;
            }
        }
    } else {
        running = false;
    }

    //Get trace
    params.clear();
    if (tracesAsInt) {
        params.append("true"); //Get traces as ints
        *samplesType = TSampleType::TUInt16;
    } else {
        params.append("false"); //Get traces as doubles
        *samplesType = TSampleType::TReal64;
    }

    succ = plugin->runPythonFunctionAndGetStringOutput(cwId, "get_last_trace", params.count(), params, dataLen, response);

    if (!succ) {
        qDebug("Error sending the get_last_trace command.");
        return 0;
    }

    traceWaitingForRead = false;

    //Set trace size
    if (tracesAsInt) {
        *samplesPerTraceDownloaded = dataLen/sizeof(int16_t);
    } else {
        *samplesPerTraceDownloaded = dataLen/sizeof(double);
    }

    //Segmented capture unavaliable, we always have only one trace
    *tracesDownloaded = 1;

    //Overvoltage supported only by Husky
    *overvoltage = false;

    //Copy trace to buffer
    size_t maxSize = dataLen > bufferSize ? bufferSize : dataLen;
    memcpy(buffer, response.toLocal8Bit().constData(), maxSize);

    //Scale int traces to fill the whole uint16_t type
    if (tracesAsInt) {
        uint16_t * uintBuf = (uint16_t *) buffer;
        int16_t * intBuf = (int16_t *) buffer;

        for (int i = 0; i < *samplesPerTraceDownloaded; ++i){
            uintBuf[i] = intBuf[i] * 64;
        }
    }

    return maxSize;
}
