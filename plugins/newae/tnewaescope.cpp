
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

    //DEBUG - todo move to its own function
    size_t dataLen;
    QString response;
    params.clear();
    succ = plugin->runPythonFunctionAndGetStringOutput(cwId, "get_name", 0, params, dataLen, response);

    if (succ) {
        qDebug("%s", (QString("Name of connected scope: ") + QString(response)).toLocal8Bit().constData());
    }

    succ = plugin->runPythonFunctionAndGetStringOutput(cwId, "capture", 0, params, dataLen, response);
    params.clear();
    params.append("false");
    succ &= plugin->runPythonFunctionAndGetStringOutput(cwId, "get_last_trace", params.count(), params, dataLen, response);
    if (succ) {
        qDebug("%s", (QString("Last trace: ") + QString(response)).toLocal8Bit().constData());
    }
    params.clear();
    params.append("true");
    succ &= plugin->runPythonFunctionAndGetStringOutput(cwId, "get_last_trace", params.count(), params, dataLen, response);
    if (succ) {
        qDebug("%s", (QString("Last trace: ") + QString(response)).toLocal8Bit().constData());
    }

    params.clear();
    params.append("both");
    params.append("true");
    succ &= plugin->runPythonFunctionAndGetStringOutput(cwId, "vglitch_setup", params.count(), params, dataLen, response);
    if (succ) {
        qDebug("%s", (QString("Succ vglitch ") + QString(response)).toLocal8Bit().constData());
    }

    succ = plugin->getPythonParameter(cwId, "gain", response);
    if (succ) {
        qDebug("%s", (QString("Gain: ") + QString(response)).toLocal8Bit().constData());
    }

    succ = plugin->getPythonSubparameter(cwId, "adc", "timeout", response);
    if (succ) {
        qDebug("%s", (QString("Timeout: ") + QString(response)).toLocal8Bit().constData());
    }

    succ = plugin->setPythonSubparameter(cwId, "adc", "timeout", "20", response);
    if (succ) {
        qDebug("%s", (QString("Timeout: ") + QString(response)).toLocal8Bit().constData());
    }

    succ = plugin->getPythonSubparameter(cwId, "adc", "timeout", response);
    if (succ) {
        qDebug("%s", (QString("Timeout: ") + QString(response)).toLocal8Bit().constData());
    }


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
