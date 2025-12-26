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
// Petr Socha

#ifndef TRECEIVERWIDGET_H
#define TRECEIVERWIDGET_H

#include <QLineEdit>
#include <QComboBox>

#include "../../protocol/tprotocol.h"
#include "../../protocol/tprotocolcontainer.h"
#include "../receiver/treceivermodel.h"

class TReceiverWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TReceiverWidget(TReceiverModel * receiverModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);

signals:
    void protocolSelected(TProtocol protocol, TReceiverModel * receiver);

public slots:
    void setAutoreceive(bool enabled);
    void receiveBytes();

    void receiveFile(QString fileName);
    void exportFile();

    void updateDisplayedProtocols();

private slots:
    void protocolChanged(int index);

private:
    TReceiverModel * m_receiverModel;

    TProtocolContainer * m_protocolContainer;

    QLineEdit * m_bytesEdit;
    QComboBox * m_protocolComboBox;
};

#endif // TRECEIVERWIDGET_H
