#include "trandom.h"

TRandom::TRandom(): m_randomGenerators(), m_preInitParams(), m_postInitParams() { }

TRandom::~TRandom() {

    (*this).TRandom::deInit();
}

QString TRandom::getPluginName() const {
    return QString("Random");
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

void TRandom::addIODevice(QString name, QString info, bool *ok) {
    m_randomGenerators.append(new TRandomDevice(name, info));
    if(ok != nullptr) *ok = true;
}

void TRandom::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
}

QList<TIODevice *> TRandom::getIODevices() {
    return m_randomGenerators;
}

QList<TScope *> TRandom::getScopes() {
    return QList<TScope *>();
}
