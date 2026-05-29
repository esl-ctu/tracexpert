// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Petr Socha (initial author)

#ifndef SERIALPORTDEVICE_H
#define SERIALPORTDEVICE_H

#include <QSerialPortInfo>
#include <QSerialPort>
#include <QElapsedTimer>
#include "tiodevice.h"

#ifdef _WIN32

#include <windows.h>

#else

#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#endif


class TSerialPortDevice : public TIODevice {

public:

    TSerialPortDevice(QString & name, QString & info);
    TSerialPortDevice(const QSerialPortInfo &portInfo);

    virtual ~TSerialPortDevice() override;

    virtual QString getName() const override;
    virtual QString getInfo() const override;

    virtual TConfigParam getPreInitParams() const override;
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    virtual void init(bool *ok = nullptr) override;
    virtual void deInit(bool *ok = nullptr) override;

    virtual TConfigParam getPostInitParams() const override;
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual size_t writeData(const uint8_t * buffer, size_t len) override;
    virtual size_t readData(uint8_t * buffer, size_t len) override;
    virtual std::optional<size_t> availableBytes() override;

protected:

    void _openPort(bool *ok = nullptr);
    void _createPostInitParams();
    bool _validatePostInitParamsStructure(TConfigParam & params);

    bool m_createdManually;
    //QSerialPort m_port;
    QSerialPortInfo m_portInfo;
    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    qint32 m_readTimeout;
    qint32 m_writeTimeout;
    bool m_initialized;

#ifdef _WIN32

    HANDLE m_osHandle;

#else

    int m_osHandle;

#endif

};

#endif // SERIALPORTDEVICE_H
