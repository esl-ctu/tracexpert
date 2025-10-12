#include "tcpaplugin.h"

#include "tcpadevice.h"

TCPAPlugin::TCPAPlugin() {
    
}

TCPAPlugin::~TCPAPlugin() {
    (*this).TCPAPlugin::deInit();
}

QString TCPAPlugin::getName() const {
    return QString("Correlation power analysis");
}

QString TCPAPlugin::getInfo() const {
    return QString("Component for univariate correlation power analysis");
}


TConfigParam TCPAPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TCPAPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TCPAPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TCPADevice());
}

void TCPAPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TCPAPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TCPAPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TCPAPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TCPAPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TCPAPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TCPAPlugin::canAddIODevice() {
    return false;
}

bool TCPAPlugin::canAddScope() {
    return false;
}

bool TCPAPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TCPAPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TCPAPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TCPAPlugin::getAnalDevices() {
    return m_analDevices;
}

