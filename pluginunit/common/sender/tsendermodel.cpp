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

#include "tsendermodel.h"

TSenderModel::TSenderModel(TSender * sender, QObject * parent)
    : QObject(parent), m_sender(sender)
{
    m_sending = false;

    m_senderThread = new QThread();
    m_sender->moveToThread(m_senderThread);

    connect(this, &TSenderModel::sendData, m_sender, &TSender::sendData, Qt::ConnectionType::QueuedConnection);
    connect(m_sender, &TSender::dataSent, this, &TSenderModel::dataSent, Qt::ConnectionType::QueuedConnection);
    connect(m_sender, &TSender::sendFailed, this, &TSenderModel::sendFailed, Qt::ConnectionType::QueuedConnection);
    connect(m_senderThread, &QThread::finished, m_sender, &QObject::deleteLater);
    connect(m_senderThread, &QThread::finished, m_senderThread, &QObject::deleteLater);

    m_senderThread->start();
}

TSenderModel::~TSenderModel()
{
    m_senderThread->quit();
}

QString TSenderModel::name() {
    return m_sender->name();
}

QString TSenderModel::info() {
    return m_sender->info();
}

void TSenderModel::writeData(QByteArray data)
{
    if (!m_sending) {
        m_sending = true;
        emit sendData(data);
    }
    else {
        emit writeBusy();
    }
}

void TSenderModel::dataSent(QByteArray data)
{
    emit dataWritten(data);
    m_sending = false;
}

void TSenderModel::sendFailed()
{
    emit writeFailed();
    m_sending = false;
}

bool TSenderModel::isBusy()
{
    return m_sender->isBusy();
}
