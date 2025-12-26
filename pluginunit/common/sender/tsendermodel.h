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

#ifndef TSENDERMODEL_H
#define TSENDERMODEL_H

#include <QThread>

#include "tsender.h"

class TSenderModel : public QObject
{
    Q_OBJECT
public:
    explicit TSenderModel(TSender * sender, QObject * parent = nullptr);
    ~TSenderModel();

    QString name();
    QString info();

    bool isBusy();

public slots:
    void writeData(QByteArray data);

signals:
    void dataWritten(QByteArray data);
    void writeFailed();
    void writeBusy();

private slots:
    void dataSent(QByteArray data);
    void sendFailed();

private:
    TSender * m_sender;
    QThread * m_senderThread;
    bool m_sending;

signals:
    void sendData(QByteArray data);
};

#endif // TSENDERMODEL_H
