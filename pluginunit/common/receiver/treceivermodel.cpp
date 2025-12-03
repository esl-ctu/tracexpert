#include "treceivermodel.h"

TReceiverModel::TReceiverModel(TReceiver * receiver, QObject * parent)
    : QObject(parent), m_receiver(receiver)
{
    m_receiving = false;
    m_autoReceive = false;

    m_receiverThread = new QThread();
    m_receiver->moveToThread(m_receiverThread);

    connect(this, &TReceiverModel::receiveData, m_receiver, &TReceiver::receiveData, Qt::ConnectionType::QueuedConnection);
    connect(m_receiver, &TReceiver::dataReceived, this, &TReceiverModel::dataReceived, Qt::ConnectionType::QueuedConnection);
    connect(m_receiver, &TReceiver::receiveFailed, this, &TReceiverModel::receiveFailed, Qt::ConnectionType::QueuedConnection);
    connect(this, &TReceiverModel::startReceiving, m_receiver, &TReceiver::startReceiving, Qt::ConnectionType::QueuedConnection);
    connect(this, &TReceiverModel::stopReceiving, m_receiver, &TReceiver::stopReceiving, Qt::ConnectionType::DirectConnection);
    connect(m_receiverThread, &QThread::finished, m_receiver, &QObject::deleteLater);
    connect(m_receiverThread, &QThread::finished, m_receiverThread, &QObject::deleteLater);

    m_receiverThread->start();
}

TReceiverModel::~TReceiverModel()
{
    m_receiverThread->quit();
}

QString TReceiverModel::name() {
    return m_receiver->name();
}

QString TReceiverModel::info() {
    return m_receiver->info();
}

void TReceiverModel::readData(int length)
{
    if (!m_receiving) {
        m_receiving = true;
        emit receiveData(length);
    }
    else {
        emit readBusy();
    }
}

void TReceiverModel::enableAutoRead()
{
    if (!m_receiving) {
        m_receiving = true;
        emit startReceiving();
    }
    else {
        emit readBusy();
    }
}

void TReceiverModel::disableAutoRead()
{
    emit stopReceiving();
    m_receiving = false;
}

void TReceiverModel::dataReceived(QByteArray data)
{
    m_receivedData.append(data);
    emit dataRead(data);
    m_receiving = m_autoReceive;
}

void TReceiverModel::receiveFailed()
{
    emit readFailed();
    m_receiving = m_autoReceive;
}

bool TReceiverModel::isBusy()
{
    return m_receiver->isBusy();
}

size_t TReceiverModel::availableBytes()
{
    return m_receiver->availableBytes();
}

QByteArray TReceiverModel::receivedData() const
{
    return m_receivedData;
}

void TReceiverModel::clearReceivedData()
{
    m_receivedData.clear();
}
