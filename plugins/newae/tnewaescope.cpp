
#include "tnewaescope.h"

TnewaeScope::TnewaeScope(const QString & name_in, const QString & sn_in, uint8_t id_in) {
    cwId = id_in;
    sn = sn_in;
    name = name_in;
    m_initialized = false;
}

TnewaeScope::~TnewaeScope() {

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
    //TODO intialize device
    pythonProcess->write("HALT");
    pythonProcess->waitForBytesWritten();

    //Pozor, inicializovat jen jednou! Bacha na IOdevice
}

void TnewaeScope::deInit(bool *ok/* = nullptr*/){

}

TConfigParam TnewaeScope::getPostInitParams() const{
    return m_postInitParams;
}

TConfigParam TnewaeDevice::setPostInitParams(TConfigParam params){

}
