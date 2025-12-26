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

#include "tiodevicemodel.h"

#include <QCoreApplication>
#include <QVariant>

#include "tiodevicecontainer.h"
#include "../component/tcomponentmodel.h"
#include "tiodevicesender.h"
#include "tiodevicereceiver.h"

TIODeviceModel::TIODeviceModel(TIODevice * IODevice, TIODeviceContainer * parent, bool manual)
    : TProjectItem(parent->model(), parent), TDeviceModel(IODevice, parent, manual), m_IODevice(IODevice)
{
    m_typeName = "iodevice";
}

TIODeviceModel::~TIODeviceModel() {
    if (!isInit())
        return;

    TIODeviceModel::deInit();
}

void TIODeviceModel::show()
{
    emit showRequested(this);
}

bool TIODeviceModel::init()
{
    if (isInit() || !TPluginUnitModel::init()) {
        return false;
    }

    m_isInit = true;

    TSender * sender = new TIODeviceSender(m_IODevice);
    TReceiver * receiver = new TIODeviceReceiver(m_IODevice);

    m_senderModel = new TSenderModel(sender);
    m_receiverModel = new TReceiverModel(receiver);

    emit initialized(this);
    itemDataChanged();

    return true;
}

bool TIODeviceModel::deInit()
{
    m_receiverModel->disableAutoRead();
    emit m_receiverModel->stopReceiving();

    while (m_receiverModel->isBusy() || m_senderModel->isBusy()) {
        QThread::msleep(10);
    }

    if (!isInit() || !TPluginUnitModel::deInit()) {
        return false;
    }

    m_isInit = false;

    delete m_senderModel;
    delete m_receiverModel;

    m_senderModel = nullptr;
    m_receiverModel = nullptr;

    emit deinitialized(this);
    itemDataChanged();

    return true;
}

bool TIODeviceModel::remove()
{
    TComponentModel * component = dynamic_cast<TComponentModel *>(TProjectItem::parent()->parent());
    if (!component)
        return false;

    return component->removeIODevice(this);
}

void TIODeviceModel::bind(TCommon * unit)
{
    m_IODevice = static_cast<TIODevice *>(unit);
    TPluginUnitModel::bind(m_IODevice);
}

void TIODeviceModel::release()
{
    m_IODevice = nullptr;
    TPluginUnitModel::release();
}

TSenderModel * TIODeviceModel::senderModel()
{
    return m_senderModel;
}

TReceiverModel *TIODeviceModel::receiverModel()
{
    return m_receiverModel;
}
