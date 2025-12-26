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

#ifndef TRECEIVER_H
#define TRECEIVER_H

#include <QObject>

#define DATA_BLOCK_SIZE 64
#define AUTORECEIVE_DELAY_MS 20

class TReceiver : public QObject
{
    Q_OBJECT

public:
    explicit TReceiver(QObject * parent = nullptr);

    virtual QString name();
    virtual QString info();

    bool isBusy();

    virtual std::optional<size_t> availableBytes();

public slots:
    void receiveData(int length);
    void startReceiving();
    void stopReceiving();

protected:
    virtual size_t readData(uint8_t * buffer, size_t len) = 0;

private:
    bool m_stopReceiving;
    bool m_isBusy = false;

signals:
    void dataReceived(QByteArray data);
    void receiveFailed();
};

#endif // TRECEIVER_H
