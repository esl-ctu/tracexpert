#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, targetType type_in, bool createdManually_in/* = true*/){
    m_initialized = false;
    m_name = name_in;
    sn = sn_in;
    plugin = plugin_in;
    m_createdManually = createdManually_in;
    scopeParent = NULL;
    cwId = NO_CW_ID;
    type = type_in;
}

void TnewaeDevice::preparePreInitParams(){
    if (type == TARGET_NORMAL){
        m_preInitParams = TConfigParam("NewAE target " + m_name + " pre-init config", "", TConfigParam::TType::TDummy, "Nothing to set here");
        return;
    }

    if (type == TARGET_CW305) {
        m_preInitParams = TConfigParam("NewAE CW305 target " + m_name + " pre-init config", "", TConfigParam::TType::TDummy, "");
        m_preInitParams.addSubParam(TConfigParam("Alpha version! If this causes a Cadmium II leak, I'm not responsible!", "", TConfigParam::TType::TDummy, ""));
        m_preInitParams.addSubParam(TConfigParam("Bitstream", "", TConfigParam::TType::TFileName, ""));
    }
    else if (type == TARGET_CW310){
        m_preInitParams = TConfigParam("NewAE CW310 target " + m_name + " pre-init config", "", TConfigParam::TType::TDummy, "");
        m_preInitParams.addSubParam(TConfigParam("Pre-alpha version! Never tested! Use at your own risk, I do NOT own a CW310!", "", TConfigParam::TType::TDummy, ""));
    }

    m_preInitParams.addSubParam(TConfigParam("To bind a scope to this target, it needs to be autodetected at the same time s the target.\nManual config is not supported.", "", TConfigParam::TType::TDummy, ""));
    auto scopeEnum1 = TConfigParam("Bind to scope", QString(""), TConfigParam::TType::TEnum, "");
    scopeEnum1.addEnumValue("None");
    QList<TScope *> scopes = plugin->getScopes();
    for (int i = 0; i < scopes.length(); ++i){
        TnewaeScope * currentScope = (TnewaeScope *) scopes.at(i);
        QString scopeSn = currentScope->getInfo();
        scopeEnum1.addEnumValue(scopeSn);
    }
    m_preInitParams.addSubParam(scopeEnum1);

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
    //reading
    auto topPrm = m_postInitParams;
    auto prms = topPrm.getSubParams();
    for (auto it = prms.begin(); it != prms.end(); ++it) {
        if ((*it).getType() == TConfigParam::TType::TDummy) { //process subparam
            //todo
        } else if ((*it).getHint() == READ_ONLY_STRING) { //get a param that needs a function call with no args
            //todo
            bool ok, ok2;
            QString out;
            size_t len;
            QString prmName = (*it).getName();
            QList<QString> arg;
            plugin->runPythonFunctionAndGetStringOutput(cwId, prmName, 0, arg, len, out, true);
            topPrm.getSubParamByName(prmName, &ok)->setValue(out.toLower(), &ok2);
            if (!ok || !ok2) {
                topPrm.setState(TConfigParam::TState::TWarning, "Cannot read some params.");
                qDebug("%s", ("Error reading param " + prmName).toLocal8Bit().constData());
            }
        } else { //normal param (python object property)
            bool ok, ok2;
            QString out;
            QString prmName = (*it).getName();
            plugin->getPythonParameter(cwId, prmName, out, true);
            topPrm.getSubParamByName(prmName, &ok)->setValue(out.toLower(), &ok2);
            if (!ok || !ok2) {
                topPrm.setState(TConfigParam::TState::TWarning, "Cannot read some params.");
                qDebug("%s", ("Error reading param " + prmName).toLocal8Bit().constData());
            }
        }
    }

    return topPrm;

/*
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

    if (!ook1) {
        paramsIn.setState(TConfigParam::TState::TError, "Read error: Unable to obtain data from the NewAE device.");
        return paramsIn;
    }

    paramsIn.getSubParamByName("Baudrate", &ok1)->setValue(out1, &ok11);

    if (!ok1 || !ok11){
        paramsIn.setState(TConfigParam::TState::TError, "Read error: Unable to write data to the postinitparams structure.");
    }
*/
    return paramsIn;
}

void TnewaeDevice::init(bool *ok/* = nullptr*/){
    QList<TScope *> scopes = plugin->getScopes();
    TnewaeScope * matchingScope = NULL;

    if (type != TARGET_NORMAL && m_preInitParams.getSubParamByName("Bind to scope")->getValue() == "None"){
        // setup target with no scope object, do both!
        //pozor, v pythonu nemáš cw id!!! asi si musíš vygenerovat nový, koukni na addScopeAutomatically jak handlovat IDs

        /* verify that this is ok
        m_postInitParams = _createPostInitParams();
        m_postInitParams = updatePostInitParams(m_postInitParams);

        if(ok != nullptr) *ok = true;
        m_initialized = true;*/
    }

    QString snToCheck = type == TARGET_NORMAL ? sn : m_preInitParams.getSubParamByName("Bind to scope")->getValue();

    for (int i = 0; i < scopes.length(); ++i){
        TnewaeScope * currentScope = (TnewaeScope *) scopes.at(i);

        QString scopeSn = currentScope->getSn();
        if (scopeSn == snToCheck) {
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
    //params.append(QString::number(cwId));
    if (type == TARGET_CW305) {
        params.append(m_preInitParams.getSubParamByName("Bitstream")->getValue());
        plugin->packageDataForPython(cwId, "T305-SETUP", 1, params, toSend);
    } else if (type == TARGET_CW305) {
        plugin->packageDataForPython(cwId, "T310-SETUP", 0, params, toSend);
    } else {
        plugin->packageDataForPython(cwId, "T-SETUP", 0, params, toSend);
    }
    bool succ = plugin->writeToPython(cwId, toSend, true);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        qCritical("Error setting target up in Python (1)");
    }

    succ = plugin->waitForPythonTargetDone(cwId);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        qCritical("Error setting target up in Python (2)");
        return;
    }

    m_postInitParams = _createPostInitParams();
    m_postInitParams = updatePostInitParams(m_postInitParams);

    if(ok != nullptr) *ok = true;
    m_initialized = true;


}

//IMPORTANT: Do not edit the hints that say "alwaysRunFunc" (READ_ONLY_STRING)!!!
TConfigParam TnewaeDevice::_createPostInitParams(){
    TConfigParam postInitParams = TConfigParam("NewAE target " + m_name + " post-init config", "", TConfigParam::TType::TDummy, "");
    if (type == TARGET_NORMAL) {
        postInitParams.addSubParam(TConfigParam("Baudrate", "38400", TConfigParam::TType::TInt, "Baudrate for the target"));
    } else if (type == TARGET_CW305) {
        postInitParams.addSubParam(TConfigParam("INITB_state", "", TConfigParam::TType::TBool, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("get_fpga_buildtime", "", TConfigParam::TType::TString, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("is_done", "", TConfigParam::TType::TBool, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("is_programmed", "", TConfigParam::TType::TBool, READ_ONLY_STRING, true));
        postInitParams.addSubParam(TConfigParam("core_type", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(TConfigParam("crypt_rev", "", TConfigParam::TType::TString, "", true));
        postInitParams.addSubParam(TConfigParam("crypt_type", "", TConfigParam::TType::TString, "", true));
        //todo functions

        auto pll = TConfigParam("pll", "", TConfigParam::TType::TDummy, "");
        pll.addSubParam(TConfigParam("pll_enable_get", "", TConfigParam::TType::TString, READ_ONLY_STRING, true));
        //todo more
        //pll.addSubParam(TConfigParam("", "", TConfigParam::TType::TString, ""));
        postInitParams.addSubParam(pll);

    } else if (type == TARGET_CW310) {

    }
    //postInitParams.addSubParam(TConfigParam("simpleserial_last_sent", "", TConfigParam::TType::TString, "The last raw string read by a simpleserial_read* command"));
    //postInitParams.addSubParam(TConfigParam("simpleserial_last_read", "", TConfigParam::TType::TString, "The last raw string written via simpleserial_write"));

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
    m_postInitParams = updatePostInitParams(m_postInitParams);

    bool succ = true;
    if (m_initialized){
        m_initialized = false;

        QString toSend;
        QList<QString> params;
        plugin->packageDataForPython(cwId, "T-DEINI", 0, params, toSend);
        succ = plugin->writeToPython(cwId, toSend, true);
        succ &= plugin->waitForPythonTargetDone(cwId);
    }

    if(ok != nullptr) *ok = succ;
}

TConfigParam TnewaeDevice::getPostInitParams() const{
    if(!m_initialized){
        //qWarning("Device not initalized! (get)");
        return m_postInitParams;
    }

    TConfigParam params = m_postInitParams;
    return updatePostInitParams(params);
}

TConfigParam TnewaeDevice::setPostInitParams(TConfigParam params){
    if(!m_initialized){
        qWarning("Device not initalized! (set)");
        return m_postInitParams;
    }

    //TODO!!
    //bool ok = _validatePostInitParamsStructure(params);
    //if (ok) {
        m_postInitParams = updatePostInitParams(params, true);
    //} else {
    //    m_postInitParams = params;
    //    qWarning("Post init params vadiation for target not successful, nothing was stored");
    //}
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

    return len;
}

size_t TnewaeDevice::readData(uint8_t * buffer, size_t len){
    size_t size;
    bool ok = plugin->readFromTarget(cwId, &size, buffer, len);

    if (ok)
        return size;
    else
        return 0;
}
