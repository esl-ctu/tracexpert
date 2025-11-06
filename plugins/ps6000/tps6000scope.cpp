#include "tps6000scope.h"

TPS6000Scope::TPS6000Scope(const QString & name, const QString & info): m_name(name), m_info(info), m_initialized(false), m_running(false), m_handle(0), m_preTrigSamples(0), m_postTrigSamples(0), m_captures(0), m_timebase(0), m_samplingPeriod(0), m_channelEnabled(false) {
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("Serial number", m_name, TConfigParam::TType::TString, "Serial number of the Picoscope 6000 (e.g., GO021/009, AQ005/139 or VDR61/356). Leave empty for the first scope found to be opened.", false));
}

TPS6000Scope::~TPS6000Scope() {
    (*this).TPS6000Scope::deInit();
}

QString TPS6000Scope::getName() const {
    return "PicoScope 6000 " + m_name;
}

QString TPS6000Scope::getInfo() const {
    return m_info;
}

TConfigParam TPS6000Scope::getPreInitParams() const {
    return m_preInitParams;
}
TConfigParam TPS6000Scope::setPreInitParams(TConfigParam params) {

    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }

    return m_preInitParams;
}

void TPS6000Scope::_createPostInitParams() {

    m_postInitParams = TConfigParam(m_name + "configuration", "", TConfigParam::TType::TDummy, "");

    // Channel settings

    TConfigParam channelSettings = TConfigParam("Channel 1 (A)", "Disabled", TConfigParam::TType::TEnum, "Configuration for channel A");
    channelSettings.addEnumValue("Disabled");
    channelSettings.addEnumValue("Enabled");
    // Coupling+Impedance
    TConfigParam couplingParam = TConfigParam("Coupling/Impedance", "DC 1 MOhm", TConfigParam::TType::TEnum, "Channel coupling/impedance");
    couplingParam.addEnumValue("DC 1 MOhm");
    couplingParam.addEnumValue("DC 50 Ohm");
    couplingParam.addEnumValue("AC 1 MOhm");
    channelSettings.addSubParam(couplingParam);
    // Channel range
    TConfigParam rangeParam = TConfigParam("Range", "-5 V .. 5 V", TConfigParam::TType::TEnum, "Channel range; 20 V and 10 V is not available when 50 Ohm impedance is set");
    rangeParam.addEnumValue("-20 V .. 20 V"); // not available when 50 Ohm
    rangeParam.addEnumValue("-10 V .. 10 V"); // not available when 50 Ohm
    rangeParam.addEnumValue("-5 V .. 5 V");
    rangeParam.addEnumValue("-2 V .. 2 V");
    rangeParam.addEnumValue("-1 V .. 1 V");
    rangeParam.addEnumValue("-500 mV .. 500 mV");
    rangeParam.addEnumValue("-200 mV .. 200 mV");
    rangeParam.addEnumValue("-100 mV .. 100 mV");
    rangeParam.addEnumValue("-50 mV .. 50 mV");
    channelSettings.addSubParam(rangeParam);
    // Analog offset
    channelSettings.addSubParam(TConfigParam("Analogue Offset", "0", TConfigParam::TType::TReal, "A voltage to add to the input channel before digitization. The allowable range depends on the channel input range."));
    // BW limiter
    TConfigParam limiterParam = TConfigParam("Bandwidth Limiter", "Full Bandwidth", TConfigParam::TType::TEnum, "20 MHz limiter is only available on 6402/6403. 25 MHz limiter is only available on 6404.");
    limiterParam.addEnumValue("Full Bandwidth");
    limiterParam.addEnumValue("20 MHz Limiter");
    limiterParam.addEnumValue("25 MHz Limiter");
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
    TConfigParam triggerAuto = TConfigParam("Auto trigger (ms)", "0", TConfigParam::TType::TInt, "The number of milliseconds the device will wait if no trigger occurs (zero = waits forever)"); // TODO nula je nekonecno?
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

void TPS6000Scope::init(bool *ok){
    _init(ok, true);
}

void TPS6000Scope::_init(bool *ok, bool createParams){

    bool iok = false;
    PICO_STATUS picoStatus;

    TConfigParam * serialParam = m_preInitParams.getSubParamByName("Serial number", &iok);
    if(!iok){
        qWarning("Serial number parameter not found in the pre-init config.");
        if(ok != nullptr) *ok = false;
        return;
    }

    QString serial = serialParam->getValue();

    if(serial.isEmpty()){

        picoStatus = ps6000OpenUnit(&m_handle, nullptr);

    } else {

        picoStatus = ps6000OpenUnit(&m_handle, (int8_t *) serial.toStdString().c_str());

    }

    if(picoStatus == PICO_OK){

        m_initialized = true;
        if(createParams){
            _createPostInitParams();
        }
        if(ok != nullptr) *ok = true;


    } else {
        // TODO zkontrolovat chybovy stavy
        if(ok != nullptr) *ok = false;

    }

}

void TPS6000Scope::deInit(bool *ok){

    if(m_initialized == true){

        PICO_STATUS picoStatus = ps6000CloseUnit(m_handle);
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

TConfigParam TPS6000Scope::getPostInitParams() const {
    return m_postInitParams;
}

void TPS6000Scope::_setChannels() {

    bool iok;

    m_channelEnabled = false;

    for (int channel = 1; channel <= 4; channel++){

        TConfigParam * channelSettings;
        PS6000_CHANNEL psChannel;

        switch(channel){
            case 1:
                channelSettings = m_postInitParams.getSubParamByName("Channel 1 (A)", &iok);
                psChannel = PS6000_CHANNEL_A;
                break;
            case 2:
                channelSettings = m_postInitParams.getSubParamByName("Channel 2 (B)", &iok);
                psChannel = PS6000_CHANNEL_B;
                break;
            case 3:
                channelSettings = m_postInitParams.getSubParamByName("Channel 3 (C)", &iok);
                psChannel = PS6000_CHANNEL_C;
                break;
            default: // case 4
                channelSettings = m_postInitParams.getSubParamByName("Channel 4 (D)", &iok);
                psChannel = PS6000_CHANNEL_D;
                break;
        }

        if(!iok){
            qCritical("Channel settings not found in the post-init params");
            return;
        }

        // Enabled parameter

        int16_t psEnabled = (channelSettings->getValue() == "Enabled") ? true : false;
        if(channelSettings->getValue() == "Enabled"){
            m_channelEnabled = true;
        }

        // Coupling parameter

        TConfigParam * couplingSub = channelSettings->getSubParamByName("Coupling/Impedance", &iok);
        if(!iok){
            qCritical("Coupling/impedance parameter not found in the post-init params");
            return;
        }

        PS6000_COUPLING psCoupling;
        QString couplingVal = couplingSub->getValue();
        if(couplingVal == "DC 1 MOhm"){
            psCoupling = PS6000_DC_1M;
        } else if(couplingVal == "DC 50 Ohm") {
            psCoupling = PS6000_DC_50R; // +-10 and +-20 volt ranges unavailable!
        } else if(couplingVal == "AC 1 MOhm") {
            psCoupling = PS6000_AC;
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

        PS6000_RANGE psRange;
        QString rangeVal = rangeSub->getValue();

        if((rangeVal == "-20 V .. 20 V" || rangeVal == "-10 V .. 10 V") && psCoupling == PS6000_DC_50R){
            rangeVal = "-5 V .. 5 V";
            rangeSub->setValue("-5 V .. 5 V");
            rangeSub->setState(TConfigParam::TState::TWarning, "The value was reset to 5 V. The 10 V and 20 V ranges are unavailable with the DC 50 Ohm coupling!");
        }

        if(rangeVal == "-50 mV .. 50 mV") {
            psRange = PS6000_50MV;
        } else if(rangeVal == "-100 mV .. 100 mV") {
            psRange = PS6000_100MV;
        } else if(rangeVal == "-200 mV .. 200 mV") {
            psRange = PS6000_200MV;
        } else if(rangeVal == "-500 mV .. 500 mV") {
            psRange = PS6000_500MV;
        } else if(rangeVal == "-1 V .. 1 V") {
            psRange = PS6000_1V;
        } else if(rangeVal == "-2 V .. 2 V") {
            psRange = PS6000_2V;
        } else if(rangeVal == "-5 V .. 5 V") {
            psRange = PS6000_5V;
        } else if(rangeVal == "-10 V .. 10 V") {
            psRange = PS6000_10V;
        } else if(rangeVal == "-20 V .. 20 V") {
            psRange = PS6000_20V;
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

        float psOffset;
        float offsetVal = offsetSub->getValue().toFloat(&iok);
        if(!iok){
            qCritical("Cannot convert the analogue offset parameter to the float");
            return;
        }

        float minOffset;    // Analogue offset values are range-dependent
        float maxOffset;
        PICO_STATUS status = ps6000GetAnalogueOffset(m_handle, psRange, psCoupling, &maxOffset, &minOffset);
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

        PS6000_BANDWIDTH_LIMITER psLimiter;
        QString limiterVal = limiterSub->getValue();

        if(limiterVal == "Full Bandwidth") {
            psLimiter = PS6000_BW_FULL;
        } else if(limiterVal == "20 MHz Limiter") {
            psLimiter = PS6000_BW_20MHZ;
        } else if(limiterVal == "25 MHz Limiter") {
            psLimiter = PS6000_BW_25MHZ;
        } else {
            qCritical("Unexpected limiter parameter");
            return;
        }

        // Setting the channel

        status = ps6000SetChannel(m_handle, psChannel, psEnabled, psCoupling, psRange, psOffset, psLimiter);

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

    }

}


void TPS6000Scope::_setTrigger() {

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
    PS6000_CHANNEL psSource;
    if(sourceVal == "Channel 1 (A)"){
        psSource = PS6000_CHANNEL_A;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 1 (A)", &iok);
    } else if(sourceVal == "Channel 2 (B)"){
        psSource = PS6000_CHANNEL_B;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 2 (B)", &iok);
    } else if(sourceVal == "Channel 3 (C)"){
        psSource = PS6000_CHANNEL_C;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 3 (C)", &iok);
    } else if(sourceVal == "Channel 4 (D)"){
        psSource = PS6000_CHANNEL_D;
        sourceChannelSettings = m_postInitParams.getSubParamByName("Channel 4 (D)", &iok);
    } else if(sourceVal == "AUX input"){
        psSource = PS6000_TRIGGER_AUX;
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

    float psADCcount = (((float)PS6000_MAX_VALUE - (float)PS6000_MIN_VALUE) * level) + (float)PS6000_MIN_VALUE;

    // Direction

    TConfigParam * directionPar = triggerSettings->getSubParamByName("Direction", &iok);
    if(!iok){
        qCritical("Trigger direction not found in the post-init params");
        return;
    }

    QString directionVal = directionPar->getValue();
    PS6000_THRESHOLD_DIRECTION psDirection;

    if(directionVal == "Rising"){
        psDirection = PS6000_RISING;
    } else if(directionVal == "Falling"){
        psDirection = PS6000_FALLING;
    } else if(directionVal == "Rising or falling"){
        psDirection = PS6000_RISING_OR_FALLING;
    } else if(directionVal == "Above"){
        psDirection = PS6000_ABOVE;
    } else if(directionVal == "Below"){
        psDirection = PS6000_BELOW;
    } else {
        qCritical("Unexpected trigger direction parameter value.");
        return;
    }

    // Autotrigger

    TConfigParam * autotriggerPar = triggerSettings->getSubParamByName("Auto trigger (ms)", &iok);
    if(!iok){
        qCritical("Autotrigger setup not found in the post-init params");
        return;
    }

    int16_t psAutoTrigger = (int16_t)autotriggerPar->getValue().toInt();

    // Setup
    PICO_STATUS status = ps6000SetSimpleTrigger(m_handle, psEnabled, psSource, psADCcount, psDirection, 0, psAutoTrigger);
    if(status != PICO_OK){
        triggerSettings->setState(TConfigParam::TState::TError, "Failed to set the trigger.");
        qWarning("Failed to set the trigger");
        return;
    }

}

bool TCompareReal(qreal a, qreal b){
    return qAbs(a - b) * 100000. <= qMin(qAbs(a), qAbs(b));
}

void TPS6000Scope::_setTiming() {

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

    uint32_t maxMemSamples;
    PICO_STATUS status = ps6000MemorySegments(m_handle, psNoOfCaptures, &maxMemSamples);

    if(status != PICO_OK) {
        noOfCapturesPar->setState(TConfigParam::TState::TError, "Failed to segment the Picoscope memory. Maybe asking for too many captures per run?");
        qWarning("Failed to segment the Picoscope memory. Maybe asking for too many captures per run?");
        return;
    }

    status = ps6000SetNoOfCaptures(m_handle, psNoOfCaptures);

    if(status != PICO_OK) {
        noOfCapturesPar->setState(TConfigParam::TState::TError, "Setting the number of captures failed. Maybe asking for too many captures per run?");
        qWarning("Setting the number of captures failed. Maybe asking for too many captures per run?");
        return;
    }

    uint32_t requestedTimebase;

    if(samplingPeriod < 6.4e-9) {

        requestedTimebase = std::log2(samplingPeriod * 5000000000.0f);
        requestedTimebase = (requestedTimebase > 4) ? 0 : requestedTimebase; // check for overflow caused by uint(float(log2))

    } else {

        requestedTimebase = samplingPeriod * 156250000.0f + 4.0f;
        requestedTimebase = (requestedTimebase < 5) ? 5 : requestedTimebase; // check for numerical errors around the border

    }

    qreal closestSamplingPeriod = (requestedTimebase <= 4) ? (std::pow(2, requestedTimebase) / 5000000000.0f) : ((requestedTimebase - 4.0f) / 156250000.0f);
    uint32_t requestedSamples = ceil(preTrigTime / closestSamplingPeriod) + ceil(postTrigTime / closestSamplingPeriod);

    bool notSatisfied = true;
    float offeredTimeInterval;
    uint32_t offeredSamples;

    uint32_t realTimebase = requestedTimebase;
    uint32_t realSamples = requestedSamples;
    qreal realSamplingPeriod = closestSamplingPeriod;

    while(realSamples > maxMemSamples){
        realTimebase += 1;
        realSamplingPeriod = (realTimebase <= 4) ? (std::pow(2, realTimebase) / 5000000000.0f) : ((realTimebase - 4.0f) / 156250000.0f);
        realSamples = ceil(preTrigTime / realSamplingPeriod) + ceil(postTrigTime / realSamplingPeriod);
    }

    do {

        notSatisfied = false;

        status = ps6000GetTimebase2(m_handle, realTimebase, realSamples, &offeredTimeInterval, 0, &offeredSamples, 0);

        if(status != PICO_OK || !TCompareReal(((qreal)offeredTimeInterval  / 1000000000.0f), realSamplingPeriod) || offeredSamples < realSamples) {

            notSatisfied = true;

            realTimebase += 1;
            realSamplingPeriod = (realTimebase <= 4) ? (std::pow(2, realTimebase) / 5000000000.0f) : ((realTimebase - 4) / 156250000.0f);
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


TConfigParam TPS6000Scope::setPostInitParams(TConfigParam params) {

    if(m_initialized == false) {
        qCritical("Cannot set post-init params on an uninitialized scope.");
    }

    m_postInitParams = params; // read the parameters
    m_postInitParams.resetState(true); // reset all warnings and errors

    ps6000Stop(m_handle); // stop the scope before changing params

    bool iok;

    deInit();
    _init(&iok, false);

    if(!iok){
        m_postInitParams.setState(TConfigParam::TState::TError, "Failed to init the scope");
        qWarning("Failed to initialize the scope during setPostInitParams");
        return m_postInitParams;
    }

    _setChannels();
    if(m_channelEnabled == true){
        _setTrigger();
        _setTiming();
    }

    return m_postInitParams;

}

void TPS6000Scope::run(size_t * expectedBufferSize, bool *ok) {

    if(m_initialized == false) {
        qCritical("Cannot run an uninitialized scope."); // or return with ok=false? ... this should never happen though
    }

    PICO_STATUS status = ps6000RunBlock(m_handle, m_preTrigSamples, m_postTrigSamples, m_timebase, 0, NULL, 0, NULL, NULL);

    if(status){
        if(ok != nullptr) *ok = false;
        qWarning("Failed to run the oscilloscope");
    } else {
        if(ok != nullptr) *ok = true;
        m_running = true;
        if(expectedBufferSize != nullptr){
            *expectedBufferSize = (m_preTrigSamples + m_postTrigSamples) * m_captures * sizeof(int16_t);
        }
        QThread::msleep(100); // wait for oscilloscope to get armed
        QThread::sleep((qreal)m_preTrigSamples * m_samplingPeriod); // wait for the preTriggerRange time, only after that the trigger can be detected
    }

}

void TPS6000Scope::stop(bool *ok) {

    if(m_initialized == false) {
        qCritical("Cannot stop an uninitialized scope."); // or return with ok=false? ... this should never happen though
    }
    //if(m_running == false){
    //    if(ok != nullptr) *ok = false; // the oscilloscope is not running, cannot stop
    //}

    ps6000Stop(m_handle);

    //if(status) {
    //    if(ok != nullptr) *ok = false;
    //    qWarning("Failed to stop the oscilloscope");
    //} else {
    //    if(ok != nullptr) *ok = true;
    //}

    // If stop occurs during data download, the ps6000Stop fails (scope aint running). But it still stops the wait for download (m_running). Indicating failure is misleading then...
    m_running = false;
    if(ok != nullptr) *ok = true; // ... so always indicate success.

}

size_t TPS6000Scope::downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage) {

    if(m_initialized == false) {
        qCritical("Cannot download traces from an uninitialized scope.");
    }

    if(channel < 1 || channel > 4) {
        qCritical("Downloading samples from an invalid channel.");
    }

    if((m_preTrigSamples + m_postTrigSamples) * m_captures * sizeof(int16_t) > bufferSize) {
        qWarning("Buffer for downloading samples from the oscilloscope is too small."); // TODO or qCritical?
        return 0;
    }

    int16_t ready = 0;
    while ((!ready) && m_running) {
        QThread::msleep(50);
        ps6000IsReady(m_handle, &ready);
    }

    ps6000IsReady(m_handle, &ready);

    if(!ready){ // the scope was stopped during waiting for the samples
        *samplesPerTraceDownloaded = 0;
        *tracesDownloaded = 0;
        return 0;
    }

    m_running = false; // the acquisition is complete

    PS6000_CHANNEL psChannel;
    switch(channel){
        case 2: psChannel = PS6000_CHANNEL_B; break;
        case 3: psChannel = PS6000_CHANNEL_C; break;
        case 4: psChannel = PS6000_CHANNEL_D; break;
        default: psChannel = PS6000_CHANNEL_A; break;
    }

    uint32_t psSamples = (m_preTrigSamples + m_postTrigSamples);
    uint32_t capturesR = m_captures;

    std::unique_ptr <int16_t> over(new int16_t[m_captures]);

    if(m_captures > 1){

        PICO_STATUS status;

        status = ps6000GetNoOfCaptures(m_handle, &capturesR);
        if (status || capturesR != m_captures){
            qWarning("Failed to capture specified number of traces per run");
        }

        for (uint32_t i = 0; i < capturesR; i++) {
            status = ps6000SetDataBufferBulk(m_handle, psChannel, reinterpret_cast<short *>(buffer) + i * psSamples, psSamples, i, PS6000_RATIO_MODE_NONE);
            if (status) {
                qWarning("Failed to set up receiving buffer");
                *samplesPerTraceDownloaded = 0;
                *tracesDownloaded = 0;
                return 0;
            }
        }

        status = ps6000GetValuesBulk(m_handle, &psSamples, 0, capturesR - 1, 0, PS6000_RATIO_MODE_NONE, over.get());
        if (status || psSamples != ((m_preTrigSamples + m_postTrigSamples))) {
            qWarning("Failed to receive the data");
            *samplesPerTraceDownloaded = 0;
            *tracesDownloaded = 0;
            return 0;
        }

        for (uint32_t i = 0; i < capturesR; i++) {
            status = ps6000SetDataBufferBulk(m_handle, psChannel, NULL, psSamples, i, PS6000_RATIO_MODE_NONE);
            if (status) {
                qWarning("Failed to release receiving buffer");
                *samplesPerTraceDownloaded = 0;
                *tracesDownloaded = 0;
                return 0;
            }
        }

    } else {

        PICO_STATUS status = ps6000SetDataBuffer(m_handle, psChannel, reinterpret_cast<short *>(buffer), m_preTrigSamples + m_postTrigSamples, PS6000_RATIO_MODE_NONE);
        if (status){
            qWarning("Failed to set up receiving buffer");
            *samplesPerTraceDownloaded = 0;
            *tracesDownloaded = 0;
            return 0;
        }

        status = ps6000GetValues(m_handle, 0, &psSamples, 1, PS6000_RATIO_MODE_NONE, 0, over.get());
        if (status || psSamples != (m_preTrigSamples + m_postTrigSamples)){
            qWarning("Failed to receive the data");
            *samplesPerTraceDownloaded = 0;
            *tracesDownloaded = 0;
            return 0;
        }

        status = ps6000SetDataBuffer(m_handle, psChannel, NULL, m_preTrigSamples + m_postTrigSamples, PS6000_RATIO_MODE_NONE);
        if (status){
            qWarning("Failed to release receiving buffer");
            *samplesPerTraceDownloaded = 0;
            *tracesDownloaded = 0;
            return 0;
        }

    }

    *overvoltage = false;
    int channel_bitidx = channel - 1;
    for(uint32_t i = 0; i < capturesR; i++){
        if( ((((over.get())[i]) >> channel_bitidx) & 0x01) > 0 ) {
            *overvoltage = true;
        }
    }

    *samplesType = TScope::TSampleType::TInt16;
    *samplesPerTraceDownloaded = psSamples;
    *tracesDownloaded = capturesR;
    return psSamples * capturesR * sizeof(int16_t);

}

qreal TPS6000Scope::rangeStrToReal(const QString & str){
    if(str == "-50 mV .. 50 mV") {
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

QList<TScope::TChannelStatus> TPS6000Scope::getChannelsStatus() {
    QList<TPS6000Scope::TChannelStatus> channelList;

    bool iok;
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


    TPS6000Scope::TChannelStatus channelA(1, "Channel A", enabledA, rangeStrToReal(rangeAVal), offsetAVal, PS6000_MIN_VALUE, PS6000_MAX_VALUE);
    TPS6000Scope::TChannelStatus channelB(2, "Channel B", enabledB, rangeStrToReal(rangeBVal), offsetBVal, PS6000_MIN_VALUE, PS6000_MAX_VALUE);
    TPS6000Scope::TChannelStatus channelC(3, "Channel C", enabledC, rangeStrToReal(rangeCVal), offsetCVal, PS6000_MIN_VALUE, PS6000_MAX_VALUE);
    TPS6000Scope::TChannelStatus channelD(4, "Channel D", enabledD, rangeStrToReal(rangeDVal), offsetDVal, PS6000_MIN_VALUE, PS6000_MAX_VALUE);


    channelList.append(channelA);
    channelList.append(channelB);
    channelList.append(channelC);
    channelList.append(channelD);

    return channelList;
}

TScope::TTimingStatus TPS6000Scope::getTimingStatus(){
    return TPS6000Scope::TTimingStatus(m_samplingPeriod, m_preTrigSamples, m_postTrigSamples, m_captures);
}

TScope::TTriggerStatus TPS6000Scope::getTriggerStatus(){
    bool iok;

    TPS6000Scope::TTriggerStatus errorStatus(TScope::TTriggerStatus::TTriggerType::TNone, 0, 0);

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

    return TPS6000Scope::TTriggerStatus(triggerType, triggerVoltage, triggerSource);

}

// + TODO getScopeTiming, getScopeTrigger methods in TScope; requires a refactoring of other plugins and GUI
// TODO zmenit TError na TWarning, kde je to potreba... pri navratu TError ve strukture je ovladani oscila zablokovany
