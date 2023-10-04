
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
}

uint8_t TnewaeScope::getId(){
    return cwId;
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
        if(!iok) return false;

        if (tmp->getValue().size() <= 0){
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
                qWarning("All other uninitalized devices need to be initialize first. Please initialize all autodetected devices and add only one manual device at a time.");
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

TnewaeScope::~TnewaeScope() {
    deInit();
}

QString TnewaeScope::getScopeName() const{
    return m_name;
}

QString TnewaeScope::getScopeInfo() const{
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

TConfigParam TnewaeScope::getPostInitParams() const{
    return m_postInitParams;
}

TConfigParam TnewaeScope::setPostInitParams(TConfigParam params){
    m_postInitParams = params;
    return m_postInitParams;
}

void TnewaeScope::run(bool *ok){
    //TODO
}
void TnewaeScope::stop(bool *ok){
    //TODO
}

size_t TnewaeScope::downloadSamples(int channel, uint8_t * buffer, size_t bufferSize,
                                    TSampleType & samplesType, size_t & samplesPerTraceDownloaded, size_t & tracesDownloaded){
    QList<double> traces;
    bool ok = plugin->getTracesFromShm(tracesDownloaded, samplesPerTraceDownloaded, traces);
    if (!ok) {
        return 0;
    }

    size_t internalSize = bufferSize / 8;
    double * internalBuffer = (double *) buffer;
    size_t i = 0;
    for (; (i < internalSize) && (i < tracesDownloaded * samplesPerTraceDownloaded); ++i){
        internalBuffer[i] = traces.at(i);
    }

    samplesType = TSampleType::TReal64;

    return i;
}
