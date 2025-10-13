#include "temptyplugin.h"

TEmptyPlugin::TEmptyPlugin() {
    
}

TEmptyPlugin::~TEmptyPlugin() {

}

QString TEmptyPlugin::getName() const {
    return QString("Empty plugin");
}

QString TEmptyPlugin::getInfo() const {
    return QString("Empty plugin info");
}


TConfigParam TEmptyPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TEmptyPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TEmptyPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TEmptyPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TEmptyPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TEmptyPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TEmptyPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TEmptyPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TEmptyPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TEmptyPlugin::canAddIODevice() {
    return true;
}

bool TEmptyPlugin::canAddScope() {
    return false;
}

bool TEmptyPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TEmptyPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TEmptyPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TEmptyPlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
