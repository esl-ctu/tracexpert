
#include "tnewaescope.h"

TnewaeScope::TnewaeScope(const QString & name_in, const QString & sn_in, uint8_t id_in, TNewae * plugin_in, bool createdManually_in) {
    m_createdManually = createdManually_in;
    m_preInitParams = TConfigParam("NewAE SN: " + name_in + " config", "", TConfigParam::TType::TDummy, "");
    _createPreInitParams();
    cwId = id_in;
    sn = sn_in;
    name = name_in;
    m_initialized = false;
    plugin = plugin_in;
}

uint8_t TnewaeScope::getId(){
    return cwId;
}

void TnewaeScope::notConnectedError() {
    qWarning("%s", (QString("NewAE device with serial number ") + QString(sn) + QString(" was disconnected. Please de-init and re-init the scope and device.")).toLocal8Bit().constData());
}

void TnewaeScope::_createPreInitParams(){
    m_preInitParams.addSubParam(TConfigParam("Serial number", QString(""), TConfigParam::TType::TString,
                                             "Serian number of the NewAE device. RO for autodetected devices.",
                                             !m_createdManually));
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
    }

    return true;
}

TnewaeScope::~TnewaeScope() {
    QString toSend;
    QList<QString> params;
    plugin->packageDataForPython(cwId, "DEINI", 0, params, toSend);
    bool succ = plugin->writeToPython(cwId, toSend);
    succ &= plugin->waitForPythonDone(cwId, true);
}

QString TnewaeScope::getScopeName() const{
    return m_name;
}

QString TnewaeScope::getScopeInfo() const{
    return m_info;
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
}

void TnewaeScope::deInit(bool *ok/* = nullptr*/){
    //TODO
}

TConfigParam TnewaeScope::getPostInitParams() const{
    return m_postInitParams;
}

TConfigParam TnewaeScope::setPostInitParams(TConfigParam params){
    //TODO
}

void TnewaeScope::run(bool *ok){
    int a = 0;
    plugin->checkForPythonState();
}
void TnewaeScope::stop(bool *ok){
    int a = 0;
    plugin->checkForPythonState();
}

size_t TnewaeScope::downloadSamples(int channel, uint8_t * buffer, size_t bufferSize,
                                    TSampleType & samplesType, size_t & samplesPerTraceDownloaded, size_t & tracesDownloaded){

}
