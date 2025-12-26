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

#ifndef TAIAPICONNECTENGINE_H
#define TAIAPICONNECTENGINE_H
#pragma once

#include "AIAPIConnectEngine_global.h"
#include "tplugin.h"
#include "taiapiconnectenginedevice.h"

class AIAPICONNECTENGINE_EXPORT TAIAPIConnectEngine : public QObject, TPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "taiapiconnectengine.json")
    Q_INTERFACES(TPlugin)

public:
    TAIAPIConnectEngine();

    virtual ~TAIAPIConnectEngine() override;

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

    /// Add an IO Device manually
    virtual TIODevice * addIODevice(QString name, QString info, bool *ok = nullptr) override;
    /// Add a Scope manually
    virtual TScope * addScope(QString name, QString info, bool *ok = nullptr) override;
    /// Add a Analytical device manually
    virtual TAnalDevice * addAnalDevice(QString name, QString info, bool *ok = nullptr) override;

    /// Returns true, when it is possible to add an IO Device manually
    virtual bool canAddIODevice() override;
    /// Returns true, when it is possible to add a Scope manually
    virtual bool canAddScope() override;
    /// Returns true, when it is possible to add an Analytical device manually
    virtual bool canAddAnalDevice() override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;
    /// Get available Analytical devices, available only after init()
    virtual QList<TAnalDevice *> getAnalDevices() override;

private:
    bool m_initialized;
    const QString PLUGIN_ID = "TraceXpert.AIAPIConnectEngine";
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    QList<TAnalDevice *> m_analDevices;

};

#endif // TAIAPICONNECTENGINE_H
