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

#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "TSerialPort_global.h"
#include "tplugin.h"
#include "tserialportdevice.h"

#include <QSerialPortInfo>


class TSERIALPORT_EXPORT TSerialPort : public QObject, TPlugin
{

    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "tserialport.json")
    Q_INTERFACES(TPlugin)

public:
    TSerialPort();
    virtual ~TSerialPort() override;

    /// Plugin name
    virtual QString getName() const override;
    /// Plugin info
    virtual QString getInfo() const override;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const override;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    /// Initialize the plugin
    virtual void init(bool *ok = nullptr) override;
    /// Deinitialize the plugin
    virtual void deInit(bool *ok = nullptr) override;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const override;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual TIODevice * addIODevice(QString name, QString info, bool *ok = nullptr) override;
    virtual TScope * addScope(QString name, QString info, bool *ok = nullptr) override;
    virtual TAnalDevice * addAnalDevice(QString name, QString info, bool *ok = nullptr) override;

    virtual bool canAddIODevice() override;
    virtual bool canAddScope() override;
    virtual bool canAddAnalDevice() override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;
    /// Get available Analytical devices, available only after init()
    virtual QList<TAnalDevice *> getAnalDevices() override;

protected:

    QList<TIODevice *> m_ports;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;

};

#endif // SERIALPORT_H
