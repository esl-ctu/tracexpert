#include "tfilescopeplugin.h"
#include "tfilescope.h"

TFileScopePlugin::TFileScopePlugin() {

}

TFileScopePlugin::~TFileScopePlugin() {

}

QString TFileScopePlugin::getName() const {
    return QString("File scope plugin");
}

QString TFileScopePlugin::getInfo() const {
    return QString("Plugin for loading and displaying traces from file");
}


TConfigParam TFileScopePlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TFileScopePlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TFileScopePlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TFileScopePlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TFileScopePlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TFileScopePlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TFileScopePlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TFileScopePlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TFileScopePlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TFileScopePlugin::canAddIODevice() {
    return false;
}

bool TFileScopePlugin::canAddScope() {
    return true;
}

bool TFileScopePlugin::canAddAnalDevice()  {
    return false;
}

QList<TIODevice *> TFileScopePlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TFileScopePlugin::getScopes() {
    auto scopeList = QList<TScope *>();
    scopeList.append(new TFileScope());
    return scopeList;
}

QList<TAnalDevice *> TFileScopePlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
