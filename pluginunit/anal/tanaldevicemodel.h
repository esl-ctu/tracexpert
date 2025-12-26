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

#ifndef TANALDEVICEMODEL_H
#define TANALDEVICEMODEL_H

#include "../common/tdevicemodel.h"
#include "tanaldevice.h"
#include "../common/receiver/treceivermodel.h"
#include "../common/sender/tsendermodel.h"
#include "action/tanalactionmodel.h"

class TAnalDeviceContainer;

class TAnalDeviceModel : public TDeviceModel
{
    Q_OBJECT

public:
    explicit TAnalDeviceModel(TAnalDevice * IODevice, TAnalDeviceContainer * parent, bool manual = false);
    ~TAnalDeviceModel();

    void show() override;

    bool init() override;
    bool deInit() override;

    bool remove() override;

    virtual void bind(TCommon * unit) override;
    virtual void release() override;

    QList<TSenderModel *> senderModels();
    QList<TReceiverModel *> receiverModels();
    QList<TAnalActionModel *> actionModels();

signals:
    void initialized(TAnalDeviceModel * analDevice);
    void deinitialized(TAnalDeviceModel * analDevice);
    void showRequested(TAnalDeviceModel * analDevice);
    void removeRequested(TAnalDeviceModel * analDevice);

private:
    TAnalDevice * m_analDevice;
    QList<TSenderModel *> m_senderModels;
    QList<TReceiverModel *> m_receiverModels;
    QList<TAnalActionModel *> m_actionModels;
};

#endif // TANALDEVICEMODEL_H
