#include "tdummyscopeplugin.h"
#include "tdummyscope.h"

TDummyScopePlugin::TDummyScopePlugin() {

}

TDummyScopePlugin::~TDummyScopePlugin() {

}

QString TDummyScopePlugin::getName() const {
    return QString("Dummy scope plugin");
}

QString TDummyScopePlugin::getInfo() const {
    return QString("Dummy scope for oscilloscope widget testing");
}


TConfigParam TDummyScopePlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TDummyScopePlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TDummyScopePlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TDummyScopePlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TDummyScopePlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TDummyScopePlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TDummyScopePlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TDummyScopePlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TDummyScopePlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TDummyScopePlugin::canAddIODevice() {
    return false;
}

bool TDummyScopePlugin::canAddScope() {
    return true;
}

bool TDummyScopePlugin::canAddAnalDevice()  {
    return false;
}

QList<TIODevice *> TDummyScopePlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TDummyScopePlugin::getScopes() {
    auto scopeList = QList<TScope *>();
    scopeList.append(new TDummyScope());
    return scopeList;
}

QList<TAnalDevice *> TDummyScopePlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
