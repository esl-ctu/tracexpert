#include "taiapiconnectengine.h"

TAIAPIConnectEngine::TAIAPIConnectEngine() {
    m_preInitParams  = TConfigParam("AI API connect engine pre-init", "", TConfigParam::TType::TDummy, "");
    m_postInitParams  = TConfigParam("AI API connect engine post-init", "", TConfigParam::TType::TDummy, "");
}

TAIAPIConnectEngine::~TAIAPIConnectEngine() {}

QString TAIAPIConnectEngine::getName() const {
    return QString("AI API Conect Engine");
}

QString TAIAPIConnectEngine::getInfo() const {
    return QString("Connects to a python AI server");
}

TConfigParam TAIAPIConnectEngine::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TAIAPIConnectEngine::setPreInitParams(TConfigParam params) {
    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

void TAIAPIConnectEngine::init(bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = true;
    m_initialized = true;
}

void TAIAPIConnectEngine::deInit(bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = true;
    m_initialized = false;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();

}

TConfigParam TAIAPIConnectEngine::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TAIAPIConnectEngine::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TAIAPIConnectEngine::addIODevice(QString name, QString info, bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TAIAPIConnectEngine::addScope(QString name, QString info, bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TAIAPIConnectEngine::addAnalDevice(QString name, QString info, bool *ok /*= nullptr*/) {
    TAnalDevice * ret = new TAIAPIConnectEngineDevice(name, info);
    m_analDevices.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;

}

bool TAIAPIConnectEngine::canAddIODevice() {
    return false;
}

bool TAIAPIConnectEngine::canAddScope() {
    return false;
}

bool TAIAPIConnectEngine::canAddAnalDevice() {
    return true;
}

QList<TIODevice *> TAIAPIConnectEngine::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TAIAPIConnectEngine::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TAIAPIConnectEngine::getAnalDevices() {
    return m_analDevices;
}
