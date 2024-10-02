#include "tanaltestingplugin.h"

#include "tanaltestingdevice.h"

TAnalTestingPlugin::TAnalTestingPlugin() {
    
}

TAnalTestingPlugin::~TAnalTestingPlugin() {

}

QString TAnalTestingPlugin::getName() const {
    return QString("Anal testing plugin");
}

QString TAnalTestingPlugin::getInfo() const {
    return QString("Plugin for testing analytical device-related features");
}


TConfigParam TAnalTestingPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TAnalTestingPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TAnalTestingPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TAnalTestingDevice());
}

void TAnalTestingPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TAnalTestingPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TAnalTestingPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TAnalTestingPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TAnalTestingPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TAnalTestingPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TAnalTestingPlugin::canAddIODevice() {
    return false;
}

bool TAnalTestingPlugin::canAddScope() {
    return false;
}

bool TAnalTestingPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TAnalTestingPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TAnalTestingPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TAnalTestingPlugin::getAnalDevices() {
    return m_analDevices;
}
