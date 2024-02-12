#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, bool createdManually_in/* = true*/){
    m_initialized = false;
    m_name = name_in;
    sn = sn_in;
    plugin = plugin_in;
    m_createdManually = createdManually_in;

    //create empty pre init params TODO
    //create post init params TODO
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
    QList<TScope *> scopes = plugin->getScopes();
    TnewaeScope * matchingScope = NULL;

    for (int i = 0; i < scopes.length(); ++i){
        TnewaeScope * currentScope = (TnewaeScope *) scopes.at(i);

        QString scopeSn = currentScope->getSn();
        if (scopeSn == sn) {
            matchingScope = currentScope;
            break;
        }
    }

    if (matchingScope) {

    } else {

    }

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
