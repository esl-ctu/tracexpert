#include "taiapiconnectenginedeviceoutputstream.h"

TAIAPIConnectEngineDeviceOutputStream::TAIAPIConnectEngineDeviceOutputStream(QString name, QString info, std::function<size_t(const uint8_t *, size_t)> writeData)
    : m_name(name), m_info(info), m_writeData(writeData) {

}

QString TAIAPIConnectEngineDeviceOutputStream::getName() const {
    return m_name;
}

QString TAIAPIConnectEngineDeviceOutputStream::getInfo() const {
    return m_info;
}

size_t TAIAPIConnectEngineDeviceOutputStream::writeData(const uint8_t * buffer, size_t len) {
    return m_writeData(buffer, len);
}
