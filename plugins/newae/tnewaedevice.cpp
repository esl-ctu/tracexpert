#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, bool createdManually_in/* = true*/){
    m_initialized = false;
    m_name = name_in;
    sn = sn_in;
    plugin = plugin_in;
    m_createdManually = createdManually_in;
    scopeParent = NULL;
    cwId = NO_CW_ID;

    m_preInitParams = TConfigParam("NewAE target " + name_in + " pre-init config", "", TConfigParam::TType::TDummy, "");
    m_postInitParams = TConfigParam("NewAE target " + name_in + " post-init config", "", TConfigParam::TType::TDummy, "");
}


TnewaeDevice::~TnewaeDevice(){
    //if(m_initialized)
    //  deInit();
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
        scopeParent = matchingScope;
        cwId = scopeParent->getId();
    } else {
        qWarning("Matching scope was not initialized. Please initialize the scope first!");
        if (ok != nullptr) {
            *ok = false;
            return;
        }
        //TODO okýnko
    }

    QString toSend;
    QList<QString> params;
    params.append(QString::number(cwId));
    plugin->packageDataForPython(cwId, "T-SETUP", 1, params, toSend);
    bool succ = plugin->writeToPython(cwId, toSend);
    succ &= plugin->waitForPythonDone(cwId, true);

    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    //TODO
    //m_postInitParams = _createPostInitParams();
    //m_postInitParams = updatePostInitParams(m_postInitParams);

    if(ok != nullptr) *ok = true;
    m_initialized = true;


}

TConfigParam TnewaeDevice::_createPostInitParams(){
    TConfigParam prms = TConfigParam("NewAE target " + m_name + " post-init config", "", TConfigParam::TType::TDummy, "");
    //TODO

    return prms;

}

bool _validatePostInitParamsStructure(TConfigParam & params){
    //TODO

    return true;
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
