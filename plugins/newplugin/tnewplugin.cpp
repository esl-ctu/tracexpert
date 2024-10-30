#include "tnewplugin.h"

TNewPlugin::TNewPlugin() {
    
}

TNewPlugin::~TNewPlugin() {

}

QString TNewPlugin::getName() const {
    return QString("New plugin");
}

QString TNewPlugin::getInfo() const {
    return QString("New plugin info");
}


TConfigParam TNewPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TNewPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TNewPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TNewPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TNewPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TNewPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TNewPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TNewPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TNewPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TNewPlugin::canAddIODevice() {
    return true;
}

bool TNewPlugin::canAddScope() {
    return false;
}

bool TNewPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TNewPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TNewPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TNewPlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
