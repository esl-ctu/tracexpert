#include "trandom.h"

TRandom::TRandom(): m_randomGenerators(), m_preInitParams(), m_postInitParams() { }

TRandom::~TRandom() {

    (*this).TRandom::deInit();
}

QString TRandom::getPluginName() const {
    return QString("Random number generator");
}

QString TRandom::getPluginInfo() const {
    return QString("Provides random number generation.");
}

TConfigParam TRandom::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TRandom::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TRandom::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TRandom::deInit(bool *ok) {
    qDeleteAll(m_randomGenerators.begin(), m_randomGenerators.end());
    m_randomGenerators.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TRandom::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TRandom::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TRandom::addIODevice(QString name, QString info, bool *ok) {
    TIODevice * ret = new TRandomDevice(name, info);
    m_randomGenerators.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TScope * TRandom::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TRandom::canAddIODevice() {
    return true;
}

bool TRandom::canAddScope() {
    return false;
}

QList<TIODevice *> TRandom::getIODevices() {
    return m_randomGenerators;
}

QList<TScope *> TRandom::getScopes() {
    return QList<TScope *>();
}
