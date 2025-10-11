#include "tcipherplugin.h"

#include "taesengine.h"

TCipherPlugin::TCipherPlugin() {
    
}

TCipherPlugin::~TCipherPlugin() {
    (*this).TCipherPlugin::deInit();
}

QString TCipherPlugin::getName() const {
    return QString("Ciphers");
}

QString TCipherPlugin::getInfo() const {
    return QString("Computes encryption/decryption intermediate values");
}


TConfigParam TCipherPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TCipherPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TCipherPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TAESEngine());
}

void TCipherPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TCipherPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TCipherPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TCipherPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TCipherPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TCipherPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TCipherPlugin::canAddIODevice() {
    return false;
}

bool TCipherPlugin::canAddScope() {
    return false;
}

bool TCipherPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TCipherPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TCipherPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TCipherPlugin::getAnalDevices() {
    return m_analDevices;
}

