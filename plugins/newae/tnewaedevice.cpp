#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, bool createdManually_in/* = true*/){
    m_initialized = false;
    m_name = name_in;
    sn = sn_in;
    plugin = plugin_in;
    m_createdManually = createdManually_in;
    scopeParent = NULL;
    cwId = NO_CW_ID;

    m_preInitParams = TConfigParam("NewAE target " + name_in + " pre-init config", "", TConfigParam::TType::TDummy, "Nothing to set here");
}


TnewaeDevice::~TnewaeDevice(){
    if(m_initialized)
        TnewaeDevice::deInit();
}

uint8_t TnewaeDevice::getId(){
    return cwId;
}

QString TnewaeDevice::getDeviceSn(){
    return sn;
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

TConfigParam TnewaeDevice::updatePostInitParams(TConfigParam paramsIn, bool write /*= false*/) const {
    if (write){
        QString out;
        bool ok1;
        plugin->setPythonParameter(cwId, "baud", paramsIn.getSubParamByName("Baudrate", &ok1)->getValue(), out, true);
        if (!ok1) {
            paramsIn.setState(TConfigParam::TState::TError, "Write error: Unable to obtain data from the postinitparams structure.");
            return paramsIn;
        }

    }

    bool ok1, ok2, ok3, ok11, ok22, ok33, ook1, ook2, ook3;
    QString out1, out2, out3;

    ook1 = plugin->getPythonParameter(cwId, "baud", out1, true);
    ook2 = plugin->getPythonParameter(cwId, "simpleserial_last_sent", out2, true);
    ook3 = plugin->getPythonParameter(cwId, "simpleserial_last_read", out3, true);

    paramsIn.getSubParamByName("baud", &ok1)->setValue(out1, &ok11);
    paramsIn.getSubParamByName("simpleserial_last_sent", &ok2)->setValue(out2, &ok22);
    paramsIn.getSubParamByName("simpleserial_last_read", &ok3)->setValue(out3, &ok33);

    if (!ook1 || !ook2 || ook3) {
        paramsIn.setState(TConfigParam::TState::TError, "Read error: Unable to obtain data from the NewAE device.");
        return paramsIn;
    }

    if (!ok1 || !ok2 || !ok3 || !ok11 || !ok22 || !ok33){
        paramsIn.setState(TConfigParam::TState::TError, "Read error: Unable to write data to the postinitparams structure.");
    }

    return paramsIn;
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
        if(!scopeParent->isInitialized()) {
            qWarning("Matching scope was found but was not initialized yet. Please initialize the scope first!");
            if (ok != nullptr) {
                //TODO okýnko
                *ok = false;
                return;
            }
        }
    } else {
        qWarning("Matching scope was not found. Please set up and initialize the scope first!");
        if (ok != nullptr) {
            *ok = false;
            //TODO okýnko
            return;
        }
    }

    QString toSend;
    QList<QString> params;
    params.append(QString::number(cwId));
    plugin->packageDataForPython(cwId, "T-SETUP", 1, params, toSend);
    bool succ = plugin->writeToPython(cwId, toSend);
    succ &= plugin->waitForPythonDone(cwId, true);

    if(!succ) {
        if(ok != nullptr) *ok = false;
        qCritical("Error setting target up in Python");
        return;
    }

    m_postInitParams = _createPostInitParams();
    m_postInitParams = updatePostInitParams(m_postInitParams);

    if(ok != nullptr) *ok = true;
    m_initialized = true;


}

TConfigParam TnewaeDevice::_createPostInitParams(){
    TConfigParam postInitParams = TConfigParam("NewAE target " + m_name + " post-init config", "", TConfigParam::TType::TDummy, "");
    postInitParams.addSubParam(TConfigParam("Baudrate", "38400", TConfigParam::TType::TInt, "Baudrate for the target"));
    postInitParams.addSubParam(TConfigParam("simpleserial_last_sent", "", TConfigParam::TType::TString, "The last raw string read by a simpleserial_read* command"));
    postInitParams.addSubParam(TConfigParam("simpleserial_last_read", "", TConfigParam::TType::TString, "The last raw string written via simpleserial_write"));

    return postInitParams;

}

bool TnewaeDevice::_validatePostInitParamsStructure(TConfigParam & params){
    bool ok = true, ok2;

    int val = params.getSubParamByName("Baudrate")->getValue().toInt(&ok2);

    if (!(val >= 500 && val <= 2000000))
        ok = false;

    if (!ok || !ok2)
        params.getSubParamByName("Baudrate")->setState(TConfigParam::TState::TWarning);

    return ok;
}

void TnewaeDevice::deInit(bool *ok/* = nullptr*/){
    m_initialized = false;

    QString toSend;
    QList<QString> params;
    plugin->packageDataForPython(cwId, "T-DEINI", 0, params, toSend);
    bool succ = plugin->writeToPython(cwId, toSend);
    succ &= plugin->waitForPythonTargetDone(cwId);

    if(ok != nullptr) *ok = succ;
}

TConfigParam TnewaeDevice::getPostInitParams() const{
    TConfigParam params = m_postInitParams;
    return updatePostInitParams(params);
}

TConfigParam TnewaeDevice::setPostInitParams(TConfigParam params){
    bool ok = _validatePostInitParamsStructure(params);
    if (ok) {
        m_postInitParams = updatePostInitParams(params);
    } else {
        m_postInitParams = params;
        qWarning("Post init params vadiation for target not successful, nothing was stored");
    }
    return m_postInitParams;
}

size_t TnewaeDevice::writeData(const uint8_t * buffer, size_t len){
    if (len == 0)
        return 0;

    QString toSend = QString::fromLocal8Bit((char *) buffer, len);
    QString out;
    QList<QString> prms;
    prms.append(toSend);
    size_t lenOut;
    bool ok = plugin->runPythonFunctionAndGetStringOutput(cwId, "write", 1, prms, lenOut, out, true);

    if (!ok)
        return 0;

    return lenOut;
}

size_t TnewaeDevice::readData(uint8_t * buffer, size_t len){
    size_t size;
    bool ok = plugin->readFromTarget(cwId, &size, buffer, len);

    if (ok)
        return size;
    else
        return 0;
}
