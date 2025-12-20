#include "taiapiconnectenginedeviceinputstream.h"

TAIAPIConnectEngineDeviceInputStream::TAIAPIConnectEngineDeviceInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData, std::function<std::optional<size_t>()> availableBytes)
    : m_name(name), m_info(info), m_readData(readData), m_availableBytes(availableBytes) {

}

QString TAIAPIConnectEngineDeviceInputStream::getName() const {
    return m_name;
}

QString TAIAPIConnectEngineDeviceInputStream::getInfo() const {
    return m_info;
}

size_t TAIAPIConnectEngineDeviceInputStream::readData(uint8_t * buffer, size_t len) {
    return m_readData(buffer, len);
}

std::optional<size_t> TAIAPIConnectEngineDeviceInputStream::availableBytes() {
    return m_availableBytes();
}
