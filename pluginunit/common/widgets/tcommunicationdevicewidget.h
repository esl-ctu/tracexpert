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

#ifndef TCOMMUNICATIONDEVICEWIDGET_H
#define TCOMMUNICATIONDEVICEWIDGET_H

#include "../../protocol/tprotocolcontainer.h"
#include "../../anal/tanaldevicemodel.h"
#include "../../io/tiodevicemodel.h"
#include "../../../common/widgets/tconfigparamwidget.h"

class TCommunicationDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TCommunicationDeviceWidget(TAnalDeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    explicit TCommunicationDeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);

public slots:
    bool applyPostInitParam();

private:
    void init(TProtocolContainer * protocolContainer);

    TDeviceModel * m_deviceModel;

    QList<TSenderModel *> m_senderModels;
    QList<TReceiverModel *> m_receiverModels;
    QList<TAnalActionModel *> m_actionModels;

    TConfigParamWidget * m_paramWidget;
};

#endif // TCOMMUNICATIONDEVICEWIDGET_H
