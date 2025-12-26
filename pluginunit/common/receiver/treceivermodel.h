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

#ifndef TRECEIVERMODEL_H
#define TRECEIVERMODEL_H

#include <QThread>

#include "treceiver.h"

class TReceiverModel : public QObject
{
    Q_OBJECT

public:
    explicit TReceiverModel(TReceiver * receiver, QObject * parent = nullptr);
    ~TReceiverModel();

    QString name();
    QString info();

    bool isBusy();

    std::optional<size_t> availableBytes();

    QByteArray receivedData() const;

public slots:
    void readData(int length);

    void enableAutoRead();
    void disableAutoRead();

    void clearReceivedData();

signals:
    void dataRead(QByteArray data);
    void readFailed();
    void readBusy();

private slots:
    void dataReceived(QByteArray data);
    void receiveFailed();

private:
    TReceiver * m_receiver;
    QThread * m_receiverThread;
    bool m_receiving;
    bool m_autoReceive;

    QByteArray m_receivedData;

signals:
    void receiveData(int length);
    void startReceiving();
    void stopReceiving();
};

#endif // TRECEIVERMODEL_H
