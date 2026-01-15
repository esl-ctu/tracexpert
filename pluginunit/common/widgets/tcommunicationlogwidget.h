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

#ifndef TCOMMUNICATIONLOGWIDGET_H
#define TCOMMUNICATIONLOGWIDGET_H

#include <QPlainTextEdit>
#include <QComboBox>

#include "../receiver/treceivermodel.h"
#include "../sender/tsendermodel.h"
#include "../protocol/tprotocol.h"
#include "../../tpalette.h"

#define DISPLAY_DATA_LENGTH_LIMIT 300

class TCommunicationLogWidget : public QWidget
{
    Q_OBJECT

public:    
    enum Direction { Sent, Received };

    explicit TCommunicationLogWidget(QList<TSenderModel *> senderModels, QList<TReceiverModel *> receiverModels, QWidget * parent = nullptr);

    void append(QString header, QString message = QString(), QColor color = QGuiApplication::palette().color(QPalette::Text));
    void appendCommunication(Direction direction, QString data, qsizetype size = -1, QString origin = QString());

    QByteArray receivedData(TReceiverModel * receiverModel);

public slots:
    void receiveBusy();
    void receiveFailed();
    void dataReceived(QByteArray data, TReceiverModel * receiverModel);

    void sendBusy();
    void sendFailed();
    void dataSent(QByteArray data, TSenderModel * senderModel);

    void messageSent(TMessage message, TSenderModel * sender);
    void protocolChanged(TProtocol protocol, TReceiverModel * receiver);

protected:
    bool event(QEvent * event);

private:
    QString byteArraytoHumanReadableString(const QByteArray & byteArray);
    void updateHtmlColors();

    QPlainTextEdit * m_textEdit;
    QComboBox * m_format;

    QList<TSenderModel *> m_senderModels;
    QList<TReceiverModel *> m_receiverModels;

    QMap<TSenderModel *, TMessage> m_sentMessages;
    QMap<TReceiverModel *, TProtocol> m_protocols;

    QColor receivedColor = TPalette::color(TPalette::CommunicationLogReceivedHighlight);
    QColor sentColor = TPalette::color(TPalette::CommunicationLogSentHighlight);
};

#endif // TCOMMUNICATIONLOGWIDGET_H
