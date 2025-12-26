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
// Tomáš Přeučil (initial author)

#ifndef TNEWAEDEVICE_H
#define TNEWAEDEVICE_H
#pragma once

#include "tnewae_global.h"


#include "tiodevice.h"

#include <QElapsedTimer>
#include <QMutex>

class TNewae;

class TnewaeDevice : public TIODevice {

public:

    TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, targetType type_in, bool createdManually_in = true);

    virtual ~TnewaeDevice() override;

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

    void preparePreInitParams();

    void setId();
    uint8_t getId();
    QString getDeviceSn();

    bool m_initialized;

protected:
    //void _openPort(bool *ok = nullptr);
    TConfigParam _createPostInitParams();
    bool _validatePostInitParamsStructure(TConfigParam & params);
    TConfigParam updatePostInitParams(TConfigParam paramsIn, bool write = false) const;

    bool m_createdManually;
    //
    //

    QString sn;
    uint8_t cwId;
    QString m_name;
    QString m_info;
    TNewae * plugin;
    TnewaeScope * scopeParent;
    targetType type;

    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    qint32 m_readTimeout;
    qint32 m_writeTimeout;

    const QString READ_ONLY_STRING = "alwaysRunFunc";
    const QString WRITE_ONLY_STRING = "writeOnlyFunc";

    const qint64 TIMER_READ_INTERVAL = 250;
    QByteArray m_readBuffer;       // dynamic buffer
    QElapsedTimer m_lastReadTimer;
    QMutex m_readMutex;
    void performHardwareRead();

};

#endif // TNEWAEDEVICE_H
