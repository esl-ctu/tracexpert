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

#ifndef TIODEVICEMODEL_H
#define TIODEVICEMODEL_H

#include <QThread>

#include "../common/tdevicemodel.h"
#include "tiodevice.h"
#include "../common/sender/tsendermodel.h"
#include "../common/receiver/treceivermodel.h"

class TIODeviceContainer;

class TIODeviceModel : public TDeviceModel
{
    Q_OBJECT

public:
    explicit TIODeviceModel(TIODevice * IODevice, TIODeviceContainer * parent, bool manual = false);
    ~TIODeviceModel();

    void show() override;

    bool init() override;
    bool deInit() override;

    bool remove() override;

    virtual void bind(TCommon * unit) override;
    virtual void release() override;

    TSenderModel * senderModel();
    TReceiverModel * receiverModel();

signals:
    void initialized(TIODeviceModel * IODevice);
    void deinitialized(TIODeviceModel * IODevice);
    void showRequested(TIODeviceModel * IODevice);
    void removeRequested(TIODeviceModel * IODevice);

private:
    TIODevice * m_IODevice;
    TSenderModel * m_senderModel = nullptr;
    TReceiverModel * m_receiverModel = nullptr;
};

#endif // TIODEVICEMODEL_H
