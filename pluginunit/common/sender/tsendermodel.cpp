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
