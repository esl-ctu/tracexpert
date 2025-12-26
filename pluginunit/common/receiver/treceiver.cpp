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

#include <QCoreApplication>
#include <QThread>

#include "treceiver.h"

TReceiver::TReceiver(QObject * parent)
    : QObject(parent)
{

}

QString TReceiver::name() {
    return QString();
}

QString TReceiver::info() {
    return QString();
}

void TReceiver::receiveData(int length)
{
    size_t remainingBytes = (size_t)length;
    quint8 buffer[DATA_BLOCK_SIZE];
    QByteArray data;

    while (remainingBytes > 0) {
        size_t bytesToReceive = (remainingBytes > DATA_BLOCK_SIZE ? DATA_BLOCK_SIZE : remainingBytes);

        size_t bytesReceived = readData(buffer, bytesToReceive);

        data.append((char*)buffer, bytesReceived);

        if (bytesReceived < bytesToReceive) {
            emit receiveFailed();
            break;
        }

        remainingBytes -= bytesReceived;
    }

    if (!data.isEmpty()) {
        emit dataReceived(data);
    }
}

void TReceiver::startReceiving()
{
    m_isBusy = true;
    m_stopReceiving = false;

    quint8 buffer[DATA_BLOCK_SIZE];

    while (!m_stopReceiving) {
        size_t newBytes = readData(buffer, DATA_BLOCK_SIZE);

        QByteArray data((char*)buffer, newBytes);

        if (!data.isEmpty()) {
            emit dataReceived(data);
        }

        QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents);
        thread()->msleep(AUTORECEIVE_DELAY_MS);
    }

    m_isBusy = false;
}

void TReceiver::stopReceiving()
{
    m_stopReceiving = true;
}

bool TReceiver::isBusy() {
    return m_isBusy;
}

std::optional<size_t> TReceiver::availableBytes() {
    return 0;
}
