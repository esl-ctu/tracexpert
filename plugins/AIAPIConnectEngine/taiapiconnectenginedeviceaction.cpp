#include "taiapiconnectenginedeviceaction.h"

TAIAPIConnectEngineDeviceAction::TAIAPIConnectEngineDeviceAction(QString name, QString info, std::function<void(void)> run)
    : m_name(name), m_info(info), m_run(run) {

}

QString TAIAPIConnectEngineDeviceAction::getName() const {
    return m_name;
}

QString TAIAPIConnectEngineDeviceAction::getInfo() const {
    return m_info;
}

bool TAIAPIConnectEngineDeviceAction::isEnabled() const {
    return true; //todo - check server availablity
}

void TAIAPIConnectEngineDeviceAction::run() {
    m_run();
}

void TAIAPIConnectEngineDeviceAction::abort() {

}

