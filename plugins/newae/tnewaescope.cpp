
#include "tnewaescope.h"

TnewaeScope::TnewaeScope(const QString & name_in, const QString & sn_in, uint8_t id_in, TNewae * plugin_in) {
    cwId = id_in;
    sn = sn_in;
    name = name_in;
    m_initialized = false;
    plugin = plugin_in;
}

TnewaeScope::~TnewaeScope() {

}

QString TnewaeScope::getIODeviceName() const{
    return m_name;
}

QString TnewaeScope::getIODeviceInfo() const{
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
    QString toSend;
    QList<QString> params;
    params.append(sn);
    plugin->packageDataForPython(cwId, "SETUP", 1, params, toSend);
    bool succ = plugin->writeToPython(cwId, toSend);
    succ &= plugin->waitForPythonDone(cwId, true);

    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    if(ok != nullptr) *ok = true;
}

void TnewaeScope::deInit(bool *ok/* = nullptr*/){

}

TConfigParam TnewaeScope::getPostInitParams() const{
    return m_postInitParams;
}

TConfigParam TnewaeScope::setPostInitParams(TConfigParam params){

}

void TnewaeScope::run(){

}
void TnewaeScope::stop(){

}

size_t TnewaeScope::getValues(int channel, int16_t * buffer, size_t len){

}
