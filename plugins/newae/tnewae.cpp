#include "tnewae.h"

TNewae::TNewae(): m_ports(), m_preInitParams(), m_postInitParams() {
    m_preInitParams  = TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect serial ports available", false);
}

TNewae::~TNewae() {

    (*this).TNewae::deInit();
}

QString TNewae::getPluginName() const {
    return QString("NewAE");
}

QString TNewae::getPluginInfo() const {
    return QString("Provides access to NewAE devices.");
}


TConfigParam TNewae::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TNewae::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TNewae::init(bool *ok) {
    //todo
}

void TNewae::deInit(bool *ok) {
    qDeleteAll(m_ports.begin(), m_ports.end());
    m_ports.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TNewae::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TNewae::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

void TNewae::addIODevice(QString name, QString info, bool *ok) {
    m_ports.append(new TnewaeDevice(name, info));
    if(ok != nullptr) *ok = true;
}

void TNewae::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
}

QList<TIODevice *> TNewae::getIODevices() {
    return m_ports;
}

QList<TScope *> TNewae::getScopes() {
    //todo
    return QList<TScope *>();
}
