#include "taiapiconnectenginedevice.h"

TAIAPIConnectEngineDevice::TAIAPIConnectEngineDevice(QString name, QString info) {}

TAIAPIConnectEngineDevice::~TAIAPIConnectEngineDevice() {}

QString TAIAPIConnectEngineDevice::getName() const {}

QString TAIAPIConnectEngineDevice::getInfo() const {}

TConfigParam TAIAPIConnectEngineDevice::getPreInitParams() const {}

TConfigParam TAIAPIConnectEngineDevice::setPreInitParams(TConfigParam params) {}

void TAIAPIConnectEngineDevice::init(bool *ok /*= nullptr*/) {}

void TAIAPIConnectEngineDevice::deInit(bool *ok /*= nullptr*/) {}

TConfigParam TAIAPIConnectEngineDevice::getPostInitParams() const {}

TConfigParam TAIAPIConnectEngineDevice::setPostInitParams(TConfigParam params) {}

QList<TAnalAction *> TAIAPIConnectEngineDevice::getActions() const {}

QList<TAnalInputStream *> TAIAPIConnectEngineDevice::getInputDataStreams() const {}

QList<TAnalOutputStream *> TAIAPIConnectEngineDevice::getOutputDataStreams() const {}

bool TAIAPIConnectEngineDevice::isBusy() const {}
