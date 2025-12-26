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
// Vojtěch Miškovský (initial author)

#ifndef TCOMPONENTMODEL_H
#define TCOMPONENTMODEL_H

#include <QObject>

#include "../tpluginunitmodel.h"
#include "tplugin.h"
#include "../io/tiodevicemodel.h"
#include "../scope/tscopemodel.h"
#include "../anal/tanaldevicemodel.h"
#include "../io/tiodevicecontainer.h"
#include "../scope/tscopecontainer.h"
#include "../anal/tanaldevicecontainer.h"

class TComponentContainer;

class TComponentModel : public TPluginUnitModel
{
    Q_OBJECT

public:
    explicit TComponentModel(TPlugin * plugin, TComponentContainer * parent);
    ~TComponentModel();

    bool init() override;
    bool deInit() override;

    int IODeviceCount() const;
    int scopeCount() const;
    int analDeviceCount() const;

    TIODeviceModel * IODevice(int index) const;
    TScopeModel * scope(int index) const;
    TAnalDeviceModel * analDevice(int index) const;

    bool canAddIODevice() const;
    bool canAddScope() const;
    bool canAddAnalDevice() const;

    bool addIODevice(QString name, QString info);
    bool addScope(QString name, QString info);
    bool addAnalDevice(QString name, QString info);

    bool removeIODevice(TIODeviceModel * IODevice);
    bool removeScope(TScopeModel * scope);
    bool removeAnalDevice(TAnalDeviceModel * analDevice);

    TIODeviceContainer * IODeviceContainer() const;
    TScopeContainer * scopeContainer() const;
    TAnalDeviceContainer * analDeviceContainer() const;

    int childrenCount() const override;
    TProjectItem * child(int row) const override;

    virtual void load(QDomElement * element) override;

signals:
    void IODeviceInitialized(TIODeviceModel * IODevice);
    void IODeviceDeinitialized(TIODeviceModel * IODevice);

    void scopeInitialized(TScopeModel * scope);
    void scopeDeinitialized(TScopeModel * scope);

    void analDeviceInitialized(TAnalDeviceModel * analDevice);
    void analDeviceDeinitialized(TAnalDeviceModel * analDevice);

private:
    void appendIODevice(TIODevice * IODevice, bool manual = false, QDomElement * element = nullptr);
    void appendScope(TScope * scope, bool manual = false, QDomElement * element = nullptr);
    void appendAnalDevice(TAnalDevice * analDevice, bool manual = false, QDomElement * element = nullptr);

    void loadIODevices(QDomElement * element);
    void loadIODevice(QDomElement * element);
    void loadScopes(QDomElement * element);
    void loadScope(QDomElement * element);
    void loadAnalDevices(QDomElement * element);
    void loadAnalDevice(QDomElement * element);

    TPlugin * m_plugin;

    TIODeviceContainer * m_IOdevices;
    TScopeContainer * m_scopes;
    TAnalDeviceContainer * m_analDevices;
};

#endif // TCOMPONENTMODEL_H
