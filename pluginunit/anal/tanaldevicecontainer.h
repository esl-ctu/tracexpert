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
// Adam Švehla

#ifndef TANALDEVICECONTAINER_H
#define TANALDEVICECONTAINER_H

#include "../tpluginunitcontainer.h"
#include "tanaldevicemodel.h"

class TComponentModel;

class TAnalDeviceContainer : public TPluginUnitContainer
{
    Q_OBJECT

public:
    explicit TAnalDeviceContainer(TComponentModel * parent = nullptr);

    int count() const override;
    TAnalDeviceModel * at(int index) const override;
    TAnalDeviceModel * getByName(const QString &name) const override;

    bool add(TAnalDeviceModel * unit);
    bool remove(TAnalDeviceModel * unit);

    TAnalDeviceModel * hasName(QString name) const;

    QString name() const override;

private:
    QList<TAnalDeviceModel *> m_analDevices;
};

#endif // TANALDEVICECONTAINER_H
