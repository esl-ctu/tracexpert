#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, bool createdManually_in/* = true*/){
    m_initialized = false;
    //TODO
}


TnewaeDevice::~TnewaeDevice(){
    //TODO
}

void TnewaeDevice::setId(){
//TODO
}

uint8_t TnewaeDevice::getId(){
//TODO
}

QString TnewaeDevice::getDeviceSn(){
//TODO
    return "";
}

QString TnewaeDevice::getName() const{
    return m_name;
}

QString TnewaeDevice::getInfo() const{
    return m_info;
}

TConfigParam TnewaeDevice::getPreInitParams() const{
    return m_preInitParams;
}

TConfigParam TnewaeDevice::setPreInitParams(TConfigParam params){
    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

void TnewaeDevice::init(bool *ok/* = nullptr*/){
    //najít a propojit se scope obejct
    //nastavit stejné id
}

void TnewaeDevice::deInit(bool *ok/* = nullptr*/){
    //TODO
}

TConfigParam TnewaeDevice::getPostInitParams() const{
    return m_postInitParams;
}

TConfigParam TnewaeDevice::setPostInitParams(TConfigParam params){
    //TODO
    return TConfigParam();
}

size_t TnewaeDevice::writeData(const uint8_t * buffer, size_t len){
    //TODO
    return 0;
}

size_t TnewaeDevice::readData(uint8_t * buffer, size_t len){
    //TODO
    return 0;
}
