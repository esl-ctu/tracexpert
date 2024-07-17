#include "tps6000ascope.h"

TPS6000aScope::TPS6000aScope(const QString & name, const QString & info): m_name(name), m_info(info), m_model("Unknown"), m_resolution(8), m_initialized(false), m_running(false), m_handle(0), m_preTrigSamples(0), m_postTrigSamples(0), m_captures(0), m_timebase(0), m_samplingPeriod(0), m_channelEnabled(false) {
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("Serial number", m_name, TConfigParam::TType::TString, "Serial number of the Picoscope 6000E (e.g., GO021/009, AQ005/139 or VDR61/356). Leave empty for the first scope found to be opened.", false));
    /*TConfigParam verticalRes = TConfigParam("Vertical resolution", "8-bit", TConfigParam::TType::TEnum, "Vertical sampling resolution");
    verticalRes.addEnumValue("8-bit");
    verticalRes.addEnumValue("10-bit");
    verticalRes.addEnumValue("12-bit");
    m_preInitParams.addSubParam(verticalRes);*/
}

TPS6000aScope::~TPS6000aScope() {
    (*this).TPS6000aScope::deInit();
}

QString TPS6000aScope::getName() const {
    return "PicoScope 6000 E series " + m_name;
}

QString TPS6000aScope::getInfo() const {
    return m_info;
}

TConfigParam TPS6000aScope::getPreInitParams() const {
    return m_preInitParams;
}
TConfigParam TPS6000aScope::setPreInitParams(TConfigParam params) {

    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }

    return m_preInitParams;
}

void TPS6000aScope::_createPostInitParams() {

    uint8_t channels;
    uint8_t resolution;
    uint32_t bandwidth;
    _getChannelsAndResolution(&channels, &resolution, &bandwidth);

    m_postInitParams = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");

    // Vertical resolution

    TConfigParam verticalResParam = TConfigParam("Vertical resolution", "8-bit", TConfigParam::TType::TEnum, "When using 12-bit resolution, only 2 channels can be enabled to capture data!");
    verticalResParam.addEnumValue("8-bit");
    if(resolution == 12){
        verticalResParam.addEnumValue("10-bit");
        verticalResParam.addEnumValue("12-bit");
    }
    m_postInitParams.addSubParam(verticalResParam);

    // Channel settings

    TConfigParam channelSettings = TConfigParam("Channel 1 (A)", "Disabled", TConfigParam::TType::TEnum, "Configuration for the channel");
    channelSettings.addEnumValue("Disabled");
    channelSettings.addEnumValue("Enabled");
    // Coupling+Impedance
    TConfigParam couplingParam = TConfigParam("Coupling/Impedance", "DC 1 MOhm", TConfigParam::TType::TEnum, "Channel coupling/impedance");
    couplingParam.addEnumValue("DC 1 MOhm");
    couplingParam.addEnumValue("DC 50 Ohm");
    couplingParam.addEnumValue("AC 1 MOhm");
    channelSettings.addSubParam(couplingParam);
    // Channel range
    TConfigParam rangeParam = TConfigParam("Range", "-500 mV .. 500 mV", TConfigParam::TType::TEnum, "Channel range; 20 V and 10 V is not available when 50 Ohm impedance is set");
    rangeParam.addEnumValue("-20 V .. 20 V"); // not available when 50 Ohm
    rangeParam.addEnumValue("-10 V .. 10 V"); // not available when 50 Ohm
    rangeParam.addEnumValue("-5 V .. 5 V");
    rangeParam.addEnumValue("-2 V .. 2 V");
    rangeParam.addEnumValue("-1 V .. 1 V");
    rangeParam.addEnumValue("-500 mV .. 500 mV");
    rangeParam.addEnumValue("-200 mV .. 200 mV");
    rangeParam.addEnumValue("-100 mV .. 100 mV");
    rangeParam.addEnumValue("-50 mV .. 50 mV");
    rangeParam.addEnumValue("-20 mV .. 20 mV");
    rangeParam.addEnumValue("-10 mV .. 10 mV");
    channelSettings.addSubParam(rangeParam);
    // Analog offset
    channelSettings.addSubParam(TConfigParam("Analogue Offset", "0", TConfigParam::TType::TReal, "A voltage to add to the input channel before digitization. The allowable range depends on the channel input range."));
    // BW limiter
    TConfigParam limiterParam = TConfigParam("Bandwidth Limiter", "Full Bandwidth", TConfigParam::TType::TEnum, "200 MHz limiter is only available on scopes with 750 MHz bandwith and above.");
    limiterParam.addEnumValue("Full Bandwidth");
    limiterParam.addEnumValue("20 MHz Limiter");
    if(bandwidth >= 750){
        limiterParam.addEnumValue("200 MHz Limiter");
    }
    channelSettings.addSubParam(limiterParam);


    // Channel 1/A
    m_postInitParams.addSubParam(channelSettings);
    // Channel 2/B
    channelSettings.setName("Channel 2 (B)");
    m_postInitParams.addSubParam(channelSettings);
    // Channel 3/C
    channelSettings.setName("Channel 3 (C)");
    m_postInitParams.addSubParam(channelSettings);
    // Channel 4/D
    channelSettings.setName("Channel 4 (D)");
    m_postInitParams.addSubParam(channelSettings);

    if(channels == 8) {
        // Channel 5/E
        channelSettings.setName("Channel 5 (E)");
        m_postInitParams.addSubParam(channelSettings);
        // Channel 6/F
        channelSettings.setName("Channel 6 (F)");
        m_postInitParams.addSubParam(channelSettings);
        // Channel 7/G
        channelSettings.setName("Channel 7 (G)");
        m_postInitParams.addSubParam(channelSettings);
        // Channel 8/H
        channelSettings.setName("Channel 8 (H)");
        m_postInitParams.addSubParam(channelSettings);
    }

    // Trigger settings

    TConfigParam triggerSettings = TConfigParam("Trigger", "Disabled", TConfigParam::TType::TEnum, "Trigger settings");
    triggerSettings.addEnumValue("Disabled");
    triggerSettings.addEnumValue("Enabled");
    // Source
    TConfigParam triggerChannel = TConfigParam("Source", "Channel 1 (A)", TConfigParam::TType::TEnum, "The channel on which to trigger");
    triggerChannel.addEnumValue("Channel 1 (A)");
    triggerChannel.addEnumValue("Channel 2 (B)");
    triggerChannel.addEnumValue("Channel 3 (C)");
    triggerChannel.addEnumValue("Channel 4 (D)");
    if(channels == 8) {
        triggerChannel.addEnumValue("Channel 5 (E)");
        triggerChannel.addEnumValue("Channel 6 (F)");
        triggerChannel.addEnumValue("Channel 7 (G)");
        triggerChannel.addEnumValue("Channel 8 (H)");
    }
    triggerChannel.addEnumValue("AUX input");
    triggerSettings.addSubParam(triggerChannel);
    // Level
    TConfigParam triggerLevel = TConfigParam("Voltage threshold", "0", TConfigParam::TType::TReal, "The voltage at which the trigger will fire");
    triggerSettings.addSubParam(triggerLevel);
    // Direction
    TConfigParam triggerDirection = TConfigParam("Direction", "Rising", TConfigParam::TType::TEnum, "The direction in which the signal must move to cause a trigger");
    triggerDirection.addEnumValue("Rising");
    triggerDirection.addEnumValue("Falling");
    triggerDirection.addEnumValue("Rising or falling");
    triggerDirection.addEnumValue("Above");
    triggerDirection.addEnumValue("Below");
    triggerSettings.addSubParam(triggerDirection);
    // Autotrigger
    TConfigParam triggerAuto = TConfigParam("Auto trigger (us)", "0", TConfigParam::TType::TInt, "The number of microseconds the device will wait if no trigger occurs (zero = waits forever)");
    triggerSettings.addSubParam(triggerAuto);
    m_postInitParams.addSubParam(triggerSettings);

    // Acquisition/Timing settings
    // Basic mode = set time and preferred samples per trace OR Advanced mode = set sampling frequency and number of samples

    TConfigParam timingSettings = TConfigParam("Timing and acquisition", "", TConfigParam::TType::TDummy, "Timing settings");
    timingSettings.addSubParam(TConfigParam("Pre-trigger time", "0", TConfigParam::TType::TTime, "The time to capture before the trigger"));
    timingSettings.addSubParam(TConfigParam("Post-trigger time", "0.002", TConfigParam::TType::TTime, "The time to capture after the trigger"));
    timingSettings.addSubParam(TConfigParam("Captures per run", "1", TConfigParam::TType::TUInt, "The number of waveform captures per a single run"));
    timingSettings.addSubParam(TConfigParam("Sampling period", "6.4e-9", TConfigParam::TType::TTime, "The time interval between samples"));
    timingSettings.addSubParam(TConfigParam("Number of samples", "312500", TConfigParam::TType::TUInt, "The number of samples per trace", true));
    m_postInitParams.addSubParam(timingSettings);

}

void TPS6000aScope::init(bool *ok){

    bool iok = false;
    PICO_STATUS picoStatus;

    TConfigParam * serialParam = m_preInitParams.getSubParamByName("Serial number", &iok);
    if(!iok){
        qWarning("Serial number parameter not found in the pre-init config.");
        if(ok != nullptr) *ok = false;
        return;
    }

    QString serial = serialParam->getValue();

    /*TConfigParam * verResParam = m_preInitParams.getSubParamByName("Vertical resolution", &iok);
    if(!iok){
        qWarning("Vertical sampling resolution parameter not found in the pre-init config.");
        if(ok != nullptr) *ok = false;
        return;
    }

    QString verResStr = verResParam->getValue();
    PICO_DEVICE_RESOLUTION tbsResolution = PICO_DR_8BIT;
    if(verResStr == "8-bit"){
        tbsResolution = PICO_DR_8BIT;
    } else if(verResStr == "10-bit"){
        tbsResolution = PICO_DR_10BIT;
    } else if(verResStr == "12-bit"){
        tbsResolution = PICO_DR_12BIT;
    } else {
        qCritical("Unexpected vertical resolution parameter value");
    }*/

    if(serial.isEmpty()){

        picoStatus = ps6000aOpenUnit(&m_handle, nullptr, PICO_DR_8BIT); // always init to 8-bit resolution, allow change later

    } else {

        picoStatus = ps6000aOpenUnit(&m_handle, (int8_t *) serial.toStdString().c_str(), PICO_DR_8BIT);

    }

    if(picoStatus == PICO_OK && m_handle > 0){

        m_initialized = true;

        std::unique_ptr<int8_t[]> info(new int8_t[1024]);
        int16_t info_len = 1024;
        int16_t req_info_len = 1024;
        picoStatus = ps6000aGetUnitInfo(m_handle, info.get(), info_len, &req_info_len, PICO_VARIANT_INFO);

        QString model((char*) info.get());
        m_model = model;

        _createPostInitParams();

        if(picoStatus == PICO_OK){
            if(ok != nullptr) *ok = true;
        } else {
            if(ok != nullptr) *ok = false;
        }

    } else {
        // TODO zkontrolovat chybovy stavy
        if(ok != nullptr) *ok = false;
    }

}

void TPS6000aScope::_getChannelsAndResolution(uint8_t * channels, uint8_t * resolution, uint32_t * bandwidth){
    if(m_model == "6403E"){
        *channels = 4;
        *resolution = 8;
        *bandwidth = 300;
    } else if(m_model == "6404E"){
        *channels = 4;
        *resolution = 8;
        *bandwidth = 500;
    } else if(m_model == "6405E"){
        *channels = 4;
        *resolution = 8;
        *bandwidth = 750;
    } else if(m_model == "6406E"){
        *channels = 4;
        *resolution = 8;
        *bandwidth = 1000;
    } else if(m_model == "6424E"){
        *channels = 4;
        *resolution = 12;
        *bandwidth = 500;
    } else if(m_model == "6425E"){
        *channels = 4;
        *resolution = 12;
        *bandwidth = 750;
    } else if(m_model == "6426E"){
        *channels = 4;
        *resolution = 12;
        *bandwidth = 1000;
    } else if(m_model == "6428E-D"){
        *channels = 4;
        *resolution = 12;
        *bandwidth = 3000;
    } else if(m_model == "6804E"){
        *channels = 8;
        *resolution = 8;
        *bandwidth = 500;
    } else if(m_model == "6824E"){
        *channels = 8;
        *resolution = 12;
        *bandwidth = 500;
    } else {
        *channels = 0;
        *resolution = 0;
        *bandwidth = 0;
        qInfo(m_model.toStdString().c_str());
        qCritical("Unsupported Picoscope model");
    }
}

void TPS6000aScope::deInit(bool *ok){

    if(m_initialized == true){

        PICO_STATUS picoStatus = ps6000aCloseUnit(m_handle);
        m_initialized = false;

        if(picoStatus != PICO_OK){
            qWarning("Error occured while closing the Picoscope");
        } else {
            if(ok != nullptr) *ok = true;
        }

    } else {
        if(ok != nullptr) *ok = false;
    }

}

TConfigParam TPS6000aScope::getPostInitParams() const {
    return m_postInitParams;
}

void TPS6000aScope::_setChannels() {

    bool iok;

    uint8_t channels;
    uint8_t resolution;
    uint32_t bandwidth;
    _getChannelsAndResolution(&channels, &resolution, &bandwidth);

    // TODO set vertical resolution

    m_channelEnabled = false;

    for (int channel = 1; channel <= channels; channel++){

        TConfigParam * channelSettings;
        PICO_CHANNEL psChannel;

        switch(channel){
            case 1:
                channelSettings = m_postInitParams.getSubParamByName("Channel 1 (A)", &iok);
                psChannel = PICO_CHANNEL_A;
                break;
            case 2:
                channelSettings = m_postInitParams.getSubParamByName("Channel 2 (B)", &iok);
                psChannel = PICO_CHANNEL_B;
                break;
            case 3:
                channelSettings = m_postInitParams.getSubParamByName("Channel 3 (C)", &iok);
                psChannel = PICO_CHANNEL_C;
                break;                
            case 4:
                channelSettings = m_postInitParams.getSubParamByName("Channel 4 (D)", &iok);
                psChannel = PICO_CHANNEL_D;
                break;
            case 5:
                channelSettings = m_postInitParams.getSubParamByName("Channel 5 (E)", &iok);
                psChannel = PICO_CHANNEL_E;
                break;
            case 6:
                channelSettings = m_postInitParams.getSubParamByName("Channel 6 (F)", &iok);
                psChannel = PICO_CHANNEL_F;
                break;
            case 7:
                channelSettings = m_postInitParams.getSubParamByName("Channel 7 (G)", &iok);
                psChannel = PICO_CHANNEL_G;
                break;
            default: // case 8
                channelSettings = m_postInitParams.getSubParamByName("Channel 8 (H)", &iok);
                psChannel = PICO_CHANNEL_H;
                break;
        }

        if(!iok){
            qCritical("Channel settings not found in the post-init params");
            return;
        }

        // Enabled parameter

        int16_t psEnabled = (channelSettings->getValue() == "Enabled") ? true : false;

        if(psEnabled == false){
            PICO_STATUS status = ps6000aSetChannelOff(m_handle, psChannel);
            if(status != PICO_OK){
                qWarning("Failed to turn off the channel");
            }
            continue; // continue with the next channel
        }

        // Coupling parameter

        TConfigParam * couplingSub = channelSettings->getSubParamByName("Coupling/Impedance", &iok);
        if(!iok){
            qCritical("Coupling/impedance parameter not found in the post-init params");
            return;
        }

        PICO_COUPLING psCoupling;
        QString couplingVal = couplingSub->getValue();
        if(couplingVal == "DC 1 MOhm"){
            psCoupling = PICO_DC;
        } else if(couplingVal == "DC 50 Ohm") {
            psCoupling = PICO_DC_50OHM; // +-10 and +-20 volt ranges unavailable!
        } else if(couplingVal == "AC 1 MOhm") {
            psCoupling = PICO_AC;
        } else {
            qCritical("Unexpected coupling/impedance parameter value.");
            return;
        }

        // Range parameter

        TConfigParam * rangeSub = channelSettings->getSubParamByName("Range", &iok);
        if(!iok){
            qCritical("Range parameter not found in the post-init params");
            return;
        }

        PICO_CONNECT_PROBE_RANGE psRange;
        QString rangeVal = rangeSub->getValue();

        if((rangeVal == "-20 V .. 20 V" || rangeVal == "-10 V .. 10 V") && psCoupling == PICO_DC_50OHM){
            rangeVal = "-5 V .. 5 V";
            rangeSub->setValue("-5 V .. 5 V");
            rangeSub->setState(TConfigParam::TState::TWarning, "The value was reset to 5 V. The 10 V and 20 V ranges are unavailable with the DC 50 Ohm coupling!");
        }

        if((rangeVal == "-10 mV .. 10 mV" || rangeVal == "-20 mV .. 20 mV") && m_model == "6428E-D"){
            rangeVal = "-50 mV .. 50 mV";
            rangeSub->setValue("-50 mV .. 50 mV");
            rangeSub->setState(TConfigParam::TState::TWarning, "The value was reset to 50 mV. The lower ranges are unavailable with the 6428E-D model!");
        }


        if((rangeVal == "-20 V .. 20 V" || rangeVal == "-10 V .. 10 V" || rangeVal == "-5 V .. 5 V" || rangeVal == "-2 V .. 2 V" || rangeVal == "-1 V .. 1 V") && m_model == "6428E-D"){
            rangeVal = "-500 mV .. 500 mV";
            rangeSub->setValue("-500 mV .. 500 mV");
            rangeSub->setState(TConfigParam::TState::TWarning, "The value was reset to 500 mV. The higher ranges are unavailable with the 6428E-D model!");
        }

        if(rangeVal == "-10 mV .. 10 mV") {
            psRange = PICO_10MV;
        } else if(rangeVal == "-20 mV .. 20 mV") {
            psRange = PICO_20MV;
        } else if(rangeVal == "-50 mV .. 50 mV") {
            psRange = PICO_50MV;
        } else if(rangeVal == "-100 mV .. 100 mV") {
            psRange = PICO_100MV;
        } else if(rangeVal == "-200 mV .. 200 mV") {
            psRange = PICO_200MV;
        } else if(rangeVal == "-500 mV .. 500 mV") {
            psRange = PICO_500MV;
        } else if(rangeVal == "-1 V .. 1 V") {
            psRange = PICO_1V;
        } else if(rangeVal == "-2 V .. 2 V") {
            psRange = PICO_2V;
        } else if(rangeVal == "-5 V .. 5 V") {
            psRange = PICO_5V;
        } else if(rangeVal == "-10 V .. 10 V") {
            psRange = PICO_10V;
        } else if(rangeVal == "-20 V .. 20 V") {
            psRange = PICO_20V;
        } else {
            qCritical("Unexpected range parameter value.");
            return;
        }


        // Offset parameter

        TConfigParam * offsetSub = channelSettings->getSubParamByName("Analogue Offset", &iok);
        if(!iok){
            qCritical("Offset parameter not found in the post-init params");
            return;
        }

        double psOffset;
        double offsetVal = offsetSub->getValue().toFloat(&iok);
        if(!iok){
            qCritical("Cannot convert the analogue offset parameter to the float");
            return;
        }

        double minOffset;    // Analogue offset values are range-dependent
        double maxOffset;
        PICO_STATUS status = ps6000aGetAnalogueOffsetLimits(m_handle, psRange, psCoupling, &maxOffset, &minOffset);
        if(status != PICO_OK){
            qCritical("Cannot get analogue offset range: Picoscope error");
            return;
        }

        if(offsetVal < minOffset){
            offsetSub->setValue(minOffset);
            offsetSub->setState(TConfigParam::TState::TWarning, "Too small offset. The value was reset to the minimum allowable value.");
            psOffset = minOffset;
        } else if(offsetVal > maxOffset) {
            offsetSub->setValue(maxOffset);
            offsetSub->setState(TConfigParam::TState::TWarning, "Too big offset. The value was reset to the maximum allowable value.");
            psOffset = maxOffset;
        } else {
            psOffset = offsetVal;
        }

        // Limiter parameter

        TConfigParam * limiterSub = channelSettings->getSubParamByName("Bandwidth Limiter", &iok);
        if(!iok){
            qCritical("Limiter parameter not found in the post-init params");
            return;
        }

        PICO_BANDWIDTH_LIMITER psLimiter;
        QString limiterVal = limiterSub->getValue();

        if(limiterVal == "Full Bandwidth") {
            psLimiter = PICO_BW_FULL;
        } else if(limiterVal == "20 MHz Limiter") {
            psLimiter = PICO_BW_20MHZ;
        } else if(limiterVal == "200 MHz Limiter") {
            psLimiter = PICO_BW_200MHZ;
        } else {
            qCritical("Unexpected limiter parameter");
            return;
        }

        // Setting the channel

        status = ps6000aSetChannelOn(m_handle, psChannel, psCoupling, psRange, psOffset, psLimiter);

        if(status == PICO_INVALID_VOLTAGE_RANGE) {
            rangeSub->setState(TConfigParam::TState::TError, "Scope error: invalid voltage range");
        } else if (status == PICO_INVALID_COUPLING) {
            couplingSub->setState(TConfigParam::TState::TError, "Scope error: invalid coupling");
        } else if (status == PICO_COUPLING_NOT_SUPPORTED) {
            couplingSub->setState(TConfigParam::TState::TError, "Scope error: coupling not supported");
        } else if (status == PICO_INVALID_ANALOGUE_OFFSET) {
            offsetSub->setState(TConfigParam::TState::TError, "Scope error: invalid offset");
        } else if (status == PICO_INVALID_BANDWIDTH) {
            limiterSub->setState(TConfigParam::TState::TError, "Scope error: invalid bandwidth");
        } else if (status == PICO_BANDWIDTH_NOT_SUPPORTED) {
            limiterSub->setState(TConfigParam::TState::TError, "Scope error: bandwidth not supported");
        } else if (status != PICO_OK) {
            qCritical("Uncaught scope error during channel setup.");
            return;
        }

        m_channelEnabled = true;

    }

}


void TPS6000aScope::_setTrigger() {

    bool iok;

    TConfigParam * triggerSettings = m_postInitParams.getSubParamByName("Trigger", &iok);

    if(!iok){
        qCritical("Trigger settings not found in the post-init params");
        return;
    }

    // Enabled

    int16_t psEnabled = (triggerSettings->getValue() == "Enabled") ? true : false;

    // Source

    TConfigParam * sourcePar = triggerSettings->getSubParamByName("Source", &iok);
    if(!iok){
        qCritical("Trigger source not found in the post-init params");
        return;
    }

    TConfigParam * sourceChannelSettings;

    QString sourceVal = sourcePar->getValue();
    PICO_CHANNEL psSource;
    if(sourceVal == "Channel 1 (A)"){
        psSource = PICO_CHANNEL_A;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 1 (A)", &iok);
    } else if(sourceVal == "Channel 2 (B)"){
        psSource = PICO_CHANNEL_B;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 2 (B)", &iok);
    } else if(sourceVal == "Channel 3 (C)"){
        psSource = PICO_CHANNEL_C;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 3 (C)", &iok);
    } else if(sourceVal == "Channel 4 (D)"){
        psSource = PICO_CHANNEL_D;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 4 (D)", &iok);
    } else if(sourceVal == "Channel 5 (E)"){
        psSource = PICO_CHANNEL_E;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 5 (E)", &iok);
    } else if(sourceVal == "Channel 6 (F)"){
        psSource = PICO_CHANNEL_F;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 6 (F)", &iok);
    } else if(sourceVal == "Channel 7 (G)"){
        psSource = PICO_CHANNEL_G;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 7 (G)", &iok);
    } else if(sourceVal == "Channel 8 (H)"){
        psSource = PICO_CHANNEL_H;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 8 (H)", &iok);
    } else if(sourceVal == "AUX input"){
        psSource = PICO_TRIGGER_AUX;
        sourceChannelSettings = nullptr;
        iok = true;
    } else {
        qCritical("Unexpected trigger source parameter");
        return;
    }
    if(!iok){
        qCritical("Failed to obtain the trigger source channel settings");
        return;
    }


    // Level

    TConfigParam * voltagePar = triggerSettings->getSubParamByName("Voltage threshold", &iok);
    if(!iok){
        qCritical("Trigger voltage threshold not found in the post-init params");
        return;
    }

    double voltageVal = voltagePar->getValue().toFloat(&iok);
    if(!iok){
        qCritical("Failed to convert the trigger voltage threshold to float");
        return;
    }

    double sourceRange = 1; // AUX input range is +- 1 V
    qreal sourceOffset  = 0;

    if(sourceChannelSettings != nullptr){
        TConfigParam * sourceRangePar = sourceChannelSettings->getSubParamByName("Range", &iok);
        if(!iok){
            qCritical("Failed to find the trigger source channel range");
            return;
        }
        QString sourceRangeVal = sourceRangePar->getValue();
        sourceRange = rangeStrToReal(sourceRangeVal);
        if(sourceRange == 0) return; 

        TConfigParam * sourceOffsetPar = sourceChannelSettings->getSubParamByName("Analogue Offset", &iok);
        if(!iok){
            qCritical("Failed to find the trigger source channel range");
            return;
        }

        sourceOffset = sourceOffsetPar->getValue().toDouble(&iok);
        if(!iok){
            qCritical("Failed to convert source offset");
            return;
        }

    }

    float level = (voltageVal + sourceOffset + sourceRange) / (2*sourceRange); // TODO zkontrolovat offset

    if(level < 0) {
        level = 0;
        voltagePar->setValue((-1)*sourceRange);
        voltagePar->setState(TConfigParam::TState::TError, "The threshold got set to the lowest possible value: voltage threshold was outside the channel range.");
    } else if (level > 1) {
        level = 1;
        voltagePar->setValue(sourceRange);
        voltagePar->setState(TConfigParam::TState::TError, "The threshold got set to the highest possible value: voltage threshold was outside the channel range.");
    }

    PICO_DEVICE_RESOLUTION psResolution = PICO_DR_8BIT;
    if(m_resolution == 8){
        psResolution = PICO_DR_8BIT;
    } else if (m_resolution == 10){
        psResolution = PICO_DR_10BIT;
    } else if (m_resolution == 12){
        psResolution = PICO_DR_12BIT;
    } else {
        qCritical("Unexpected vertical resolution value");
    }
    int16_t psMinVal;
    int16_t psMaxVal;
    PICO_STATUS status = ps6000aGetAdcLimits(m_handle, psResolution, &psMinVal, &psMaxVal);
    if(status != PICO_OK){
        qCritical("Failed to obtain ADC limits");
    }
    float psADCcount = (((float)psMaxVal - (float)psMinVal) * level) + (float)psMinVal;

    // Direction

    TConfigParam * directionPar = triggerSettings->getSubParamByName("Direction", &iok);
    if(!iok){
        qCritical("Trigger direction not found in the post-init params");
        return;
    }

    QString directionVal = directionPar->getValue();
    PICO_THRESHOLD_DIRECTION psDirection;

    if(directionVal == "Rising"){
        psDirection = PICO_RISING;
    } else if(directionVal == "Falling"){
        psDirection = PICO_FALLING;
    } else if(directionVal == "Rising or falling"){
        psDirection = PICO_RISING_OR_FALLING;
    } else if(directionVal == "Above"){
        psDirection = PICO_ABOVE;
    } else if(directionVal == "Below"){
        psDirection = PICO_BELOW;
    } else {
        qCritical("Unexpected trigger direction parameter value.");
        return;
    }

    // Autotrigger

    TConfigParam * autotriggerPar = triggerSettings->getSubParamByName("Auto trigger (us)", &iok);
    if(!iok){
        qCritical("Autotrigger setup not found in the post-init params");
        return;
    }

    int32_t psAutoTrigger = (int32_t)autotriggerPar->getValue().toInt();

    // Setup
    status = ps6000aSetSimpleTrigger(m_handle, psEnabled, psSource, psADCcount, psDirection, 0, psAutoTrigger);
    if(status != PICO_OK){
        triggerSettings->setState(TConfigParam::TState::TError, "Failed to set the trigger.");
        qWarning("Failed to set the trigger");
        return;
    }

}

bool TCompareReal(qreal a, qreal b){
    return qAbs(a - b) * 100000. <= qMin(qAbs(a), qAbs(b));
}

uint32_t TPS6000aScope::_getTimebase(qreal samplingPeriod){

    uint32_t timebase;

    if(m_model == "6428E-D"){

        if(samplingPeriod < 6.4e-9) {

            timebase = std::log2(samplingPeriod * 10000000000.0f);
            timebase = (timebase > 5) ? 0 : timebase; // check for overflow caused by uint(float(log2))

        } else {

            timebase = samplingPeriod * 156250000.0f + 5.0f;
            timebase = (timebase < 6) ? 6 : timebase; // check for numerical errors around the border

        }

    } else {

        if(samplingPeriod < 6.4e-9) {

            timebase = std::log2(samplingPeriod * 5000000000.0f);
            timebase = (timebase > 4) ? 0 : timebase; // check for overflow caused by uint(float(log2))

        } else {

            timebase = samplingPeriod * 156250000.0f + 4.0f;
            timebase = (timebase < 5) ? 5 : timebase; // check for numerical errors around the border

        }

    }

    return timebase;
}
qreal TPS6000aScope::_getSamplingPeriod(uint32_t timeBase){

    if(m_model == "6428E-D"){

        return (timeBase <= 5) ? (std::pow(2, timeBase) / 10000000000.0f) : ((timeBase - 5.0f) / 156250000.0f);

    } else {

        return (timeBase <= 4) ? (std::pow(2, timeBase) / 5000000000.0f) : ((timeBase - 4.0f) / 156250000.0f);

    }

}

void TPS6000aScope::_setTiming() {

    bool iok;

    TConfigParam * timingSettings = m_postInitParams.getSubParamByName("Timing and acquisition", &iok);
    if(!iok){
        qCritical("Timing settings not found in the post-init params");
        return;
    }

    qreal preTrigTime;

    TConfigParam * preTrigTimePar = timingSettings->getSubParamByName("Pre-trigger time", &iok);
    if(!iok){
        qCritical("Pre-trigger time not found in the post-init params");
        return;
    }
    preTrigTime = preTrigTimePar->getValue().toDouble(&iok);
    if(!iok){
        qCritical("Failed to convert pre-trigger time");
        return;
    }

    qreal postTrigTime;

    TConfigParam * postTrigTimePar = timingSettings->getSubParamByName("Post-trigger time", &iok);
    if(!iok){
        qCritical("Post-trigger time not found in the post-init params");
        return;
    }
    postTrigTime = postTrigTimePar->getValue().toDouble(&iok);
    if(!iok){
        qCritical("Failed to convert post-trigger time");
        return;
    }

    qreal samplingPeriod; // the requested sampling period

    TConfigParam * samplingPeriodPar = timingSettings->getSubParamByName("Sampling period", &iok);
    if(!iok){
        qCritical("Sampling period not found in the post-init params");
        return;
    }
    samplingPeriod = samplingPeriodPar->getValue().toDouble(&iok);
    if(!iok){
        qCritical("Failed to convert sampling period");
        return;
    }

    uint32_t psNoOfCaptures;

    TConfigParam * noOfCapturesPar = timingSettings->getSubParamByName("Captures per run", &iok);
    if(!iok){
        qCritical("Number of captures per run not found in the post-init params");
        return;
    }
    psNoOfCaptures = noOfCapturesPar->getValue().toUInt(&iok);
    if(!iok){
        qCritical("Failed to convert number of captures");
        return;
    }

    if(psNoOfCaptures < 1){
        noOfCapturesPar->setState(TConfigParam::TState::TError, "Number of captures must be greater of equal to 1.");
        qWarning("Number of captures must be greater of equal to 1.");
        return;
    }

    uint64_t maxMemSamples;
    PICO_STATUS status = ps6000aMemorySegments(m_handle, psNoOfCaptures, &maxMemSamples);

    if(status != PICO_OK) {
        noOfCapturesPar->setState(TConfigParam::TState::TError, "Failed to segment the Picoscope memory. Maybe asking for too many captures per run?");
        qWarning("Failed to segment the Picoscope memory. Maybe asking for too many captures per run?");
        return;
    }

    status = ps6000aSetNoOfCaptures(m_handle, psNoOfCaptures);

    if(status != PICO_OK) {
        noOfCapturesPar->setState(TConfigParam::TState::TError, "Setting the number of captures failed. Maybe asking for too many captures per run?");
        qWarning("Setting the number of captures failed. Maybe asking for too many captures per run?");
        return;
    }            

    uint32_t requestedTimebase;

    requestedTimebase = _getTimebase(samplingPeriod);

    /*if(samplingPeriod < 6.4e-9) {

        requestedTimebase = std::log2(samplingPeriod * 5000000000.0f);
        requestedTimebase = (requestedTimebase > 4) ? 0 : requestedTimebase; // check for overflow caused by uint(float(log2))

    } else {

        requestedTimebase = samplingPeriod * 156250000.0f + 4.0f;
        requestedTimebase = (requestedTimebase < 5) ? 5 : requestedTimebase; // check for numerical errors around the border

    }*/

    qreal closestSamplingPeriod = _getSamplingPeriod(requestedTimebase);
    //qreal closestSamplingPeriod = (requestedTimebase <= 4) ? (std::pow(2, requestedTimebase) / 5000000000.0f) : ((requestedTimebase - 4.0f) / 156250000.0f);

    uint32_t requestedSamples = ceil(preTrigTime / closestSamplingPeriod) + ceil(postTrigTime / closestSamplingPeriod);

    bool notSatisfied = true;
    double offeredTimeInterval;
    uint64_t offeredSamples;

    uint32_t realTimebase = requestedTimebase;
    uint64_t realSamples = requestedSamples;
    qreal realSamplingPeriod = closestSamplingPeriod;

    while(realSamples > maxMemSamples){
        realTimebase += 1;
        realSamplingPeriod = _getSamplingPeriod(realTimebase);
        //realSamplingPeriod = (realTimebase <= 4) ? (std::pow(2, realTimebase) / 5000000000.0f) : ((realTimebase - 4.0f) / 156250000.0f);
        realSamples = ceil(preTrigTime / realSamplingPeriod) + ceil(postTrigTime / realSamplingPeriod);
    }

    do {

        notSatisfied = false;

        status = ps6000aGetTimebase(m_handle, realTimebase, realSamples, &offeredTimeInterval, &offeredSamples, 0);

        if(status != PICO_OK || !TCompareReal(((qreal)offeredTimeInterval  / 1000000000.0f), realSamplingPeriod) || offeredSamples < realSamples) {

            notSatisfied = true;

            realTimebase += 1;
            realSamplingPeriod = _getSamplingPeriod(realTimebase);
            //realSamplingPeriod = (realTimebase <= 4) ? (std::pow(2, realTimebase) / 5000000000.0f) : ((realTimebase - 4) / 156250000.0f);
            realSamples = ceil(preTrigTime / realSamplingPeriod) + ceil(postTrigTime / realSamplingPeriod);

            if(realTimebase == requestedTimebase){
                samplingPeriodPar->setState(TConfigParam::TState::TError, "Failed to find a suitable sampling period.");
                qWarning("Failed to find a suitable sampling period.");
                return;
            }

        }

        if(status == PICO_INVALID_HANDLE || status == PICO_INVALID_CHANNEL || status == PICO_SEGMENT_OUT_OF_RANGE || status == PICO_DRIVER_FUNCTION) {
            samplingPeriodPar->setState(TConfigParam::TState::TError, "Failed finding a timebase. Scope error.");
            qWarning("Failed finding a timebase. Scope error.");
            return;
        }

    } while (notSatisfied);

    m_timebase = realTimebase;
    m_captures = psNoOfCaptures;
    m_preTrigSamples = ceil(preTrigTime / realSamplingPeriod);
    m_postTrigSamples = ceil(postTrigTime / realSamplingPeriod);
    m_samplingPeriod = realSamplingPeriod;

    // set post-init preTime + warning
    preTrigTimePar->setValue(m_preTrigSamples * realSamplingPeriod);
    if(m_preTrigSamples * realSamplingPeriod != preTrigTime){
        preTrigTimePar->setState(TConfigParam::TState::TWarning, "The time was adjusted according to the oscilloscope sampling period");
    }
    // set post-init postTime + warning
    postTrigTimePar->setValue(m_postTrigSamples * realSamplingPeriod);
    if(m_postTrigSamples * realSamplingPeriod != postTrigTime){
        postTrigTimePar->setState(TConfigParam::TState::TWarning, "The time was adjusted according to the oscilloscope sampling period");
    }
    // set post-init samples
    TConfigParam * samplesPar = timingSettings->getSubParamByName("Number of samples", &iok);
    if(!iok){
        qCritical("Number of captures per run not found in the post-init params");
        return;
    }
    samplesPar->setValue(m_preTrigSamples+m_postTrigSamples);
    // set sampling period + warning
    samplingPeriodPar->setValue(realSamplingPeriod);
    if(realSamplingPeriod != samplingPeriod){
        samplingPeriodPar->setState(TConfigParam::TState::TWarning, "The sampling period was adjusted to the nearest (smaller if possible) according to the oscilloscope memory capabilities and the capture time requirements");
    }

}


TConfigParam TPS6000aScope::setPostInitParams(TConfigParam params) {

    if(m_initialized == false) {
        qCritical("Cannot set post-init params on an uninitialized scope.");
    }

    m_postInitParams = params; // read the parameters
    m_postInitParams.resetState(true); // reset all warnings and errors

    bool iok;
    TConfigParam * vertResParam = m_postInitParams.getSubParamByName("Vertical resolution", &iok);

    if(!iok){
        qCritical("Vertical resolution param not found in the post-init params");
    }

    PICO_DEVICE_RESOLUTION psResolution = PICO_DR_8BIT;
    QString vertResStr = vertResParam->getValue();
    if(vertResStr == "8-bit"){
        psResolution = PICO_DR_8BIT;
        m_resolution = 8;
    } else if(vertResStr == "10-bit"){
        psResolution = PICO_DR_10BIT;
        m_resolution = 10;
    } else if(vertResStr == "12-bit"){
        psResolution = PICO_DR_12BIT;
        m_resolution = 12;
    } else {
        qCritical("Unexpected value of the vertical resolution parameter");
    }

    PICO_STATUS status = ps6000aSetDeviceResolution(m_handle, psResolution);
    if(status != PICO_OK){
        vertResParam->setState(TConfigParam::TState::TError, "Failed to set the desired resolution (out of range?).");
    }

    _setChannels();
    if(m_channelEnabled == true){
        _setTrigger();
        _setTiming();
    }

    return m_postInitParams;

}

void TPS6000aScope::run(size_t * expectedBufferSize, bool *ok) {

    if(m_initialized == false) {
        qCritical("Cannot run an uninitialized scope."); // or return with ok=false? ... this should never happen though
    }

    PICO_STATUS status = ps6000aSetNoOfCaptures(m_handle, m_captures);

    if(status){
        if(ok != nullptr) *ok = false;
        qWarning("Failed to set the number of captures prior to run");
    }

    status = ps6000aRunBlock(m_handle, m_preTrigSamples, m_postTrigSamples, m_timebase, NULL, 0, NULL, NULL);

    if(status){
        if(ok != nullptr) *ok = false;
        qWarning("Failed to run the oscilloscope");
    } else {
        if(ok != nullptr) *ok = true;
        m_running = true;
        if(expectedBufferSize != nullptr){
            *expectedBufferSize = (m_preTrigSamples + m_postTrigSamples) * m_captures * sizeof(int16_t);
        }
        QThread::msleep(100); // wait for oscilloscope to get armed // TODO re-check value for E series
        QThread::sleep((qreal)m_preTrigSamples * m_samplingPeriod); // wait for the preTriggerRange time, only after that the trigger can be detected
    }

}

void TPS6000aScope::stop(bool *ok) {

    if(m_initialized == false) {
        qCritical("Cannot stop an uninitialized scope."); // or return with ok=false? ... this should never happen though
    }
    //if(m_running == false){
    //   if(ok != nullptr) *ok = false; // the oscilloscope is not running, cannot stop
    //}

    ps6000aStop(m_handle);

    //if(status) {
        //if(ok != nullptr) *ok = false;
        //qWarning("Failed to stop the oscilloscope");
    //} else {
        //if(ok != nullptr) *ok = true;
    //}

    // If stop occurs during data download, the ps6000aStop fails (scope aint running). But it still stops the wait for download (m_running). Indicating failure is misleading then...
    m_running = false;
    if(ok != nullptr) *ok = true; // ... so always indicate success.

}

size_t TPS6000aScope::downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage) {

    uint8_t channels;
    uint8_t resolution;
    uint32_t bandwidth;
    _getChannelsAndResolution(&channels, &resolution, &bandwidth);

    if(m_initialized == false) {
        qCritical("Cannot download traces from an uninitialized scope.");
    }

    if(channel < 1 || channel > channels) {
        qCritical("Downloading samples from an invalid channel.");
    }

    if((m_preTrigSamples + m_postTrigSamples) * m_captures * sizeof(int16_t) > bufferSize) {
        qWarning("Buffer for downloading samples from the oscilloscope is too small."); // TODO or qCritical?
        return 0;
    }

    int16_t ready = 0;
    while ((!ready) && m_running) {
        QThread::msleep(50);
        ps6000aIsReady(m_handle, &ready);
    }

    ps6000aIsReady(m_handle, &ready);

    if(!ready){ // the scope was stopped during waiting for the samples
        *samplesPerTraceDownloaded = 0;
        *tracesDownloaded = 0;
        return 0;
    }

    m_running = false; // the acquisition is complete

    PICO_CHANNEL psChannel;
    switch(channel){
        case 2: psChannel = PICO_CHANNEL_B; break;
        case 3: psChannel = PICO_CHANNEL_C; break;
        case 4: psChannel = PICO_CHANNEL_D; break;
        case 5: psChannel = PICO_CHANNEL_E; break;
        case 6: psChannel = PICO_CHANNEL_F; break;
        case 7: psChannel = PICO_CHANNEL_G; break;
        case 8: psChannel = PICO_CHANNEL_H; break;
        default: psChannel = PICO_CHANNEL_A; break;
    }

    uint64_t psSamples = (m_preTrigSamples + m_postTrigSamples);

    std::unique_ptr <int16_t> over(new int16_t[m_captures]);

    uint64_t capturesR = m_captures;

    if(capturesR > 1){

        PICO_STATUS status;

        status = ps6000aGetNoOfCaptures(m_handle, &capturesR);
        if (status || capturesR != m_captures){
            qWarning((QString("Failed to capture specified number of traces per run: %1 requested, %2 got").arg(m_captures).arg(capturesR)).toStdString().c_str());
        }

        PICO_ACTION psAction = (PICO_ACTION) (PICO_CLEAR_ALL | PICO_ADD);

        for (uint64_t i = 0; i < capturesR; i++) {
            status = ps6000aSetDataBuffer(m_handle, psChannel, reinterpret_cast<short *>(buffer) + i * psSamples, psSamples, PICO_INT16_T, i, PICO_RATIO_MODE_RAW, psAction);
            if (status) {
                qWarning("Failed to set up receiving buffer");
                *samplesPerTraceDownloaded = 0;
                *tracesDownloaded = 0;
                return 0;
            }
            psAction = PICO_ADD;
        }

        status = ps6000aGetValuesBulk(m_handle, 0, &psSamples, 0, capturesR - 1, 1, PICO_RATIO_MODE_RAW, over.get());
        if (status || psSamples != ((m_preTrigSamples + m_postTrigSamples))) {
            qWarning("Failed to receive the data");
            *samplesPerTraceDownloaded = 0;
            *tracesDownloaded = 0;
            return 0;
        }

    } else {

        PICO_ACTION psAction = (PICO_ACTION) (PICO_CLEAR_ALL | PICO_ADD);

        PICO_STATUS status = ps6000aSetDataBuffer(m_handle, psChannel, reinterpret_cast<short *>(buffer), m_preTrigSamples + m_postTrigSamples, PICO_INT16_T, 0, PICO_RATIO_MODE_RAW, psAction);
        if (status){
            qWarning("Failed to set up receiving buffer");
            *samplesPerTraceDownloaded = 0;
            *tracesDownloaded = 0;
            return 0;
        }

        status = ps6000aGetValues(m_handle, 0, &psSamples, 1, PICO_RATIO_MODE_RAW, 0, over.get());
        if (status || psSamples != (m_preTrigSamples + m_postTrigSamples)){
            qWarning("Failed to receive the data");
            *samplesPerTraceDownloaded = 0;
            *tracesDownloaded = 0;
            return 0;
        }

    }

    *overvoltage = false;
    int channel_bitidx = channel - 1;
    for(uint64_t i = 0; i < capturesR; i++){
        if( ((((over.get())[i]) >> channel_bitidx) & 0x01) > 0 ) {
            *overvoltage = true;
        }
    }

    *samplesType = TScope::TSampleType::TInt16;
    *samplesPerTraceDownloaded = psSamples;
    *tracesDownloaded = capturesR;
    return psSamples * capturesR * sizeof(int16_t);

}

qreal TPS6000aScope::rangeStrToReal(const QString & str){
    if(str == "-10 mV .. 10 mV") {
        return 0.01f;
    } else if(str == "-20 mV .. 20 mV") {
        return 0.02f;
    } else if(str == "-50 mV .. 50 mV") {
        return 0.05f;
    } else if(str == "-100 mV .. 100 mV") {
        return 0.1f;
    } else if(str == "-200 mV .. 200 mV") {
        return 0.2f;
    } else if(str == "-500 mV .. 500 mV") {
        return 0.5f;
    } else if(str == "-1 V .. 1 V") {
        return 1.0f;
    } else if(str == "-2 V .. 2 V") {
        return 2.0f;
    } else if(str == "-5 V .. 5 V") {
        return 5.0f;
    } else if(str == "-10 V .. 10 V") {
        return 10.0f;
    } else if(str == "-20 V .. 20 V") {
        return 20.0f;
    } else {
        qCritical("Unexpected range parameter value.");
        return 0;
    }
}

QList<TScope::TChannelStatus> TPS6000aScope::getChannelsStatus() {

    uint8_t channels;
    uint8_t resolution;
    uint32_t bandwidth;
    _getChannelsAndResolution(&channels, &resolution, &bandwidth);

    QList<TPS6000aScope::TChannelStatus> channelList;

    bool iok;
    int16_t minAdcVal = 1;
    int16_t maxAdcVal = -1;

    TConfigParam * verResSettings = m_postInitParams.getSubParamByName("Vertical resolution", &iok);
    if(!iok){
        qCritical("Vertical resolution settings not found during channel status retrieval");
        return QList<TScope::TChannelStatus>();
    }
    QString verResVal = verResSettings->getValue();
    PICO_DEVICE_RESOLUTION psRes;
    if(verResVal == "8-bit"){
        psRes = PICO_DR_8BIT;
    } else if (verResVal == "10-bit") {
        psRes = PICO_DR_10BIT;
    } else if (verResVal == "12-bit") {
        psRes = PICO_DR_12BIT;
    } else {
        qCritical("Unexpected vertical resolution settings");
        return QList<TScope::TChannelStatus>();
    }

    PICO_STATUS psStatus = ps6000aGetAdcLimits(m_handle, psRes, &minAdcVal, &maxAdcVal);
    if(psStatus){
        qCritical("Failed to obtain ADC limits from the scope.");
        return QList<TScope::TChannelStatus>();
    }

    TConfigParam * channelASettings = m_postInitParams.getSubParamByName("Channel 1 (A)", &iok);
    if(!iok){
        qCritical("Channel 1 settings not found during channel status retrieval");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * channelBSettings = m_postInitParams.getSubParamByName("Channel 2 (B)", &iok);
    if(!iok){
        qCritical("Channel 2 settings not found during channel status retrieval");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * channelCSettings = m_postInitParams.getSubParamByName("Channel 3 (C)", &iok);
    if(!iok){
        qCritical("Channel 3 settings not found during channel status retrieval");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * channelDSettings = m_postInitParams.getSubParamByName("Channel 4 (D)", &iok);
    if(!iok){
        qCritical("Channel 4 settings not found during channel status retrieval");
        return QList<TScope::TChannelStatus>();
    }

    bool enabledA = (channelASettings->getValue() == "Enabled") ? true : false;
    bool enabledB = (channelBSettings->getValue() == "Enabled") ? true : false;
    bool enabledC = (channelCSettings->getValue() == "Enabled") ? true : false;
    bool enabledD = (channelDSettings->getValue() == "Enabled") ? true : false;

    TConfigParam * rangeASub = channelASettings->getSubParamByName("Range", &iok);
    if(!iok){
        qCritical("Range parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * rangeBSub = channelBSettings->getSubParamByName("Range", &iok);
    if(!iok){
        qCritical("Range parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * rangeCSub = channelCSettings->getSubParamByName("Range", &iok);
    if(!iok){
        qCritical("Range parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * rangeDSub = channelDSettings->getSubParamByName("Range", &iok);
    if(!iok){
        qCritical("Range parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }

    QString rangeAVal = rangeASub->getValue();
    QString rangeBVal = rangeBSub->getValue();
    QString rangeCVal = rangeCSub->getValue();
    QString rangeDVal = rangeDSub->getValue();

    TConfigParam * offsetASub = channelASettings->getSubParamByName("Analogue Offset", &iok);
    if(!iok){
        qCritical("Offset parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * offsetBSub = channelBSettings->getSubParamByName("Analogue Offset", &iok);
    if(!iok){
        qCritical("Offset parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * offsetCSub = channelCSettings->getSubParamByName("Analogue Offset", &iok);
    if(!iok){
        qCritical("Offset parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }
    TConfigParam * offsetDSub = channelDSettings->getSubParamByName("Analogue Offset", &iok);
    if(!iok){
        qCritical("Offset parameter not found in the post-init params");
        return QList<TScope::TChannelStatus>();
    }

    qreal offsetAVal = offsetASub->getValue().toFloat(&iok);
    if(!iok){
        qCritical("Cannot convert the analogue offset parameter to the float");
        return QList<TScope::TChannelStatus>();
    }
    qreal offsetBVal = offsetBSub->getValue().toFloat(&iok);
    if(!iok){
        qCritical("Cannot convert the analogue offset parameter to the float");
        return QList<TScope::TChannelStatus>();
    }
    qreal offsetCVal = offsetCSub->getValue().toFloat(&iok);
    if(!iok){
        qCritical("Cannot convert the analogue offset parameter to the float");
        return QList<TScope::TChannelStatus>();
    }
    qreal offsetDVal = offsetDSub->getValue().toFloat(&iok);
    if(!iok){
        qCritical("Cannot convert the analogue offset parameter to the float");
        return QList<TScope::TChannelStatus>();
    }


    TPS6000aScope::TChannelStatus channelA(1, "Channel A", enabledA, rangeStrToReal(rangeAVal), offsetAVal, minAdcVal, maxAdcVal);
    TPS6000aScope::TChannelStatus channelB(2, "Channel B", enabledB, rangeStrToReal(rangeBVal), offsetBVal, minAdcVal, maxAdcVal);
    TPS6000aScope::TChannelStatus channelC(3, "Channel C", enabledC, rangeStrToReal(rangeCVal), offsetCVal, minAdcVal, maxAdcVal);
    TPS6000aScope::TChannelStatus channelD(4, "Channel D", enabledD, rangeStrToReal(rangeDVal), offsetDVal, minAdcVal, maxAdcVal);


    channelList.append(channelA);
    channelList.append(channelB);
    channelList.append(channelC);
    channelList.append(channelD);

    if(channels == 8){

        TConfigParam * channelESettings = m_postInitParams.getSubParamByName("Channel 5 (E)", &iok);
        if(!iok){
            qCritical("Channel 5 settings not found during channel status retrieval");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * channelFSettings = m_postInitParams.getSubParamByName("Channel 6 (F)", &iok);
        if(!iok){
            qCritical("Channel 6 settings not found during channel status retrieval");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * channelGSettings = m_postInitParams.getSubParamByName("Channel 7 (G)", &iok);
        if(!iok){
            qCritical("Channel 7 settings not found during channel status retrieval");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * channelHSettings = m_postInitParams.getSubParamByName("Channel 8 (H)", &iok);
        if(!iok){
            qCritical("Channel 8 settings not found during channel status retrieval");
            return QList<TScope::TChannelStatus>();
        }

        bool enabledE = (channelESettings->getValue() == "Enabled") ? true : false;
        bool enabledF = (channelFSettings->getValue() == "Enabled") ? true : false;
        bool enabledG = (channelGSettings->getValue() == "Enabled") ? true : false;
        bool enabledH = (channelHSettings->getValue() == "Enabled") ? true : false;

        TConfigParam * rangeESub = channelESettings->getSubParamByName("Range", &iok);
        if(!iok){
            qCritical("Range parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * rangeFSub = channelFSettings->getSubParamByName("Range", &iok);
        if(!iok){
            qCritical("Range parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * rangeGSub = channelGSettings->getSubParamByName("Range", &iok);
        if(!iok){
            qCritical("Range parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * rangeHSub = channelHSettings->getSubParamByName("Range", &iok);
        if(!iok){
            qCritical("Range parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }

        QString rangeEVal = rangeESub->getValue();
        QString rangeFVal = rangeFSub->getValue();
        QString rangeGVal = rangeGSub->getValue();
        QString rangeHVal = rangeHSub->getValue();

        TConfigParam * offsetESub = channelESettings->getSubParamByName("Analogue Offset", &iok);
        if(!iok){
            qCritical("Offset parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * offsetFSub = channelFSettings->getSubParamByName("Analogue Offset", &iok);
        if(!iok){
            qCritical("Offset parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * offsetGSub = channelGSettings->getSubParamByName("Analogue Offset", &iok);
        if(!iok){
            qCritical("Offset parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }
        TConfigParam * offsetHSub = channelHSettings->getSubParamByName("Analogue Offset", &iok);
        if(!iok){
            qCritical("Offset parameter not found in the post-init params");
            return QList<TScope::TChannelStatus>();
        }

        qreal offsetEVal = offsetESub->getValue().toFloat(&iok);
        if(!iok){
            qCritical("Cannot convert the analogue offset parameter to the float");
            return QList<TScope::TChannelStatus>();
        }
        qreal offsetFVal = offsetFSub->getValue().toFloat(&iok);
        if(!iok){
            qCritical("Cannot convert the analogue offset parameter to the float");
            return QList<TScope::TChannelStatus>();
        }
        qreal offsetGVal = offsetGSub->getValue().toFloat(&iok);
        if(!iok){
            qCritical("Cannot convert the analogue offset parameter to the float");
            return QList<TScope::TChannelStatus>();
        }
        qreal offsetHVal = offsetHSub->getValue().toFloat(&iok);
        if(!iok){
            qCritical("Cannot convert the analogue offset parameter to the float");
            return QList<TScope::TChannelStatus>();
        }


        TPS6000aScope::TChannelStatus channelE(5, "Channel E", enabledE, rangeStrToReal(rangeEVal), offsetEVal, minAdcVal, maxAdcVal);
        TPS6000aScope::TChannelStatus channelF(6, "Channel F", enabledF, rangeStrToReal(rangeFVal), offsetFVal, minAdcVal, maxAdcVal);
        TPS6000aScope::TChannelStatus channelG(7, "Channel G", enabledG, rangeStrToReal(rangeGVal), offsetGVal, minAdcVal, maxAdcVal);
        TPS6000aScope::TChannelStatus channelH(8, "Channel H", enabledH, rangeStrToReal(rangeHVal), offsetHVal, minAdcVal, maxAdcVal);


        channelList.append(channelE);
        channelList.append(channelF);
        channelList.append(channelG);
        channelList.append(channelH);


    }

    return channelList;

}

TScope::TTimingStatus TPS6000aScope::getTimingStatus(){
    return TPS6000aScope::TTimingStatus(m_samplingPeriod, m_preTrigSamples, m_postTrigSamples, m_captures);
}

TScope::TTriggerStatus TPS6000aScope::getTriggerStatus(){
    bool iok;

    TPS6000aScope::TTriggerStatus errorStatus(TScope::TTriggerStatus::TTriggerType::TNone, 0, 0);

    TConfigParam * triggerSettings = m_postInitParams.getSubParamByName("Trigger", &iok);

    if(!iok){
        qCritical("Trigger settings not found in the post-init params");
        return errorStatus;
    }

    // Enabled

    bool triggerEnabled = (triggerSettings->getValue() == "Enabled") ? true : false;

    // Source

    TConfigParam * sourcePar = triggerSettings->getSubParamByName("Source", &iok);
    if(!iok){
        qCritical("Trigger source not found in the post-init params");
        return errorStatus;
    }

    QString sourceVal = sourcePar->getValue();
    uint32_t triggerSource;
    if(sourceVal == "Channel 1 (A)"){
        triggerSource = 1;
    } else if(sourceVal == "Channel 2 (B)"){
        triggerSource = 2;
    } else if(sourceVal == "Channel 3 (C)"){
        triggerSource = 3;
    } else if(sourceVal == "Channel 4 (D)"){
        triggerSource = 4;
    } else if(sourceVal == "Channel 5 (E)"){
        triggerSource = 5;
    } else if(sourceVal == "Channel 6 (F)"){
        triggerSource = 6;
    } else if(sourceVal == "Channel 7 (G)"){
        triggerSource = 7;
    } else if(sourceVal == "Channel 8 (H)"){
        triggerSource = 8;
    } else if(sourceVal == "AUX input"){
        triggerSource = 0;
    } else {
        qCritical("Unexpected trigger source parameter");
        return errorStatus;
    }
    if(!iok){
        qCritical("Failed to obtain the trigger source channel settings");
        return errorStatus;
    }

    // Level

    TConfigParam * voltagePar = triggerSettings->getSubParamByName("Voltage threshold", &iok);
    if(!iok){
        qCritical("Trigger voltage threshold not found in the post-init params");
        return errorStatus;
    }

    qreal triggerVoltage = voltagePar->getValue().toFloat(&iok);
    if(!iok){
        qCritical("Failed to convert the trigger voltage threshold to float");
        return errorStatus;
    }

    // Direction

    TConfigParam * directionPar = triggerSettings->getSubParamByName("Direction", &iok);
    if(!iok){
        qCritical("Trigger direction not found in the post-init params");
        return errorStatus;
    }

    QString directionVal = directionPar->getValue();
    TScope::TTriggerStatus::TTriggerType triggerType = TScope::TTriggerStatus::TTriggerType::TNone;

    if(triggerEnabled == true){
        if(directionVal == "Rising"){
            triggerType = TScope::TTriggerStatus::TTriggerType::TRising;
        } else if(directionVal == "Falling"){
            triggerType = TScope::TTriggerStatus::TTriggerType::TFalling;
        } else if(directionVal == "Rising or falling"){
            triggerType = TScope::TTriggerStatus::TTriggerType::TRisingOrFalling;
        } else if(directionVal == "Above"){
            triggerType = TScope::TTriggerStatus::TTriggerType::TAbove;
        } else if(directionVal == "Below"){
            triggerType = TScope::TTriggerStatus::TTriggerType::TBelow;
        } else {
            qCritical("Unexpected trigger direction parameter value.");
            return errorStatus;
        }
    }

    return TPS6000aScope::TTriggerStatus(triggerType, triggerVoltage, triggerSource);

}

