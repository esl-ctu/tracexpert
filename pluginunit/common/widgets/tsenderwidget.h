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

#ifndef TSENDERWIDGET_H
#define TSENDERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>

#include "../../protocol/tprotocol.h"
#include "../../protocol/tprotocolcontainer.h"
#include "tmessageformmanager.h"
#include "../sender/tsendermodel.h"

class TSenderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TSenderWidget(TSenderModel * senderModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    ~TSenderWidget();

signals:
    void messageSent(TMessage message, TSenderModel * sender);

public slots:
    void sendBytes();
    void sendRawBytes();
    void sendProtocolBytes();

    void sendFile(QString fileName);
    void importFile();

    bool validateRawInputValues();

private slots:
    void protocolChanged(int index);
    void messageChanged(int index);

    void updateDisplayedProtocols();

private:
    TSenderModel * m_senderModel;

    QComboBox * m_protocolComboBox;
    QComboBox * m_messageComboBox;
    QLabel * m_noMessagesLabel;
    QPushButton * m_sendButton;
    QFormLayout * m_formLayout;

    TProtocolContainer * m_protocolContainer;

    TMessageFormManager * m_messageFormManager;

    TProtocol m_selectedProtocol;
    TMessage m_selectedMessage;

    QLineEdit * m_rawMessageEdit;
    QComboBox * m_rawFormatComboBox;
    QHBoxLayout * m_rawMessageEditLayout;
};

#endif // TSENDERWIDGET_H
