#include "tttestplugin.h"

#include "tttestdevice.h"

TTTestPlugin::TTTestPlugin() {
    
}

TTTestPlugin::~TTTestPlugin() {
    (*this).TTTestPlugin::deInit();
}

QString TTTestPlugin::getName() const {
    return QString("Univariate Welch's t-test plugin");
}

QString TTTestPlugin::getInfo() const {
    return QString("Plugin for t-test leakage assessment");
}


TConfigParam TTTestPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TTTestPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TTTestPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TTTestDevice());
}

void TTTestPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TTTestPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TTTestPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TTTestPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TTTestPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TTTestPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TTTestPlugin::canAddIODevice() {
    return false;
}

bool TTTestPlugin::canAddScope() {
    return false;
}

bool TTTestPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TTTestPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TTTestPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TTTestPlugin::getAnalDevices() {
    return m_analDevices;
}
