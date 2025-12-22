#ifndef TAIAPICONNECTENGINEDEVICEINPUTSTREAM_H
#define TAIAPICONNECTENGINEDEVICEINPUTSTREAM_H
#pragma once

#include "tanaldevice.h"

class TAIAPIConnectEngineDeviceInputStream : public TAnalInputStream {
public:
    explicit TAIAPIConnectEngineDeviceInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData, std::function<std::optional<size_t>()> availableBytes);

    /// AnalStream name
    QString getName() const override;
    /// AnalStream info
    QString getInfo() const override;

    size_t readData(uint8_t * buffer, size_t len) override;

    std::optional<size_t> availableBytes() override;

private:
    QString m_name;
    QString m_info;
    std::function<size_t(uint8_t *, size_t)> m_readData;
    std::function<std::optional<size_t>()> m_availableBytes;

};

#endif // TAIAPICONNECTENGINEDEVICEINPUTSTREAM_H
