#include "tiodevicemodel.h"

#include <QCoreApplication>
#include <QVariant>

#include "tiodevicecontainer.h"

TIODeviceModel::TIODeviceModel(TIODevice * IODevice, TIODeviceContainer * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitModel(parent), m_IODevice(IODevice)
{
    m_sending = false;
    m_receiving = false;
    m_autoReceive = false;
}

TIODeviceModel::~TIODeviceModel()
{
    TIODeviceModel::deInit();
}

QString TIODeviceModel::name() const
{
    return m_IODevice->getIODeviceName();
}

QString TIODeviceModel::info() const
{
    return m_IODevice->getIODeviceInfo();
}

bool TIODeviceModel::init()
{
    if (isInit()) {
        return false;
    }

    bool ok;
    m_IODevice->init(&ok);

    if (ok) {
        m_isInit = true;

        m_receiver = new TIODeviceReceiver(m_IODevice);
        m_receiver->moveToThread(&receiverThread);
        m_sender = new TIODeviceSender(m_IODevice);
        m_sender->moveToThread(&senderThread);

        connect(this, &TIODeviceModel::sendData, m_sender, &TIODeviceSender::sendData, Qt::ConnectionType::QueuedConnection);
        connect(m_sender, &TIODeviceSender::dataSent, this, &TIODeviceModel::dataSent, Qt::ConnectionType::QueuedConnection);
        connect(m_sender, &TIODeviceSender::sendFailed, this, &TIODeviceModel::sendFailed, Qt::ConnectionType::QueuedConnection);

        connect(this, &TIODeviceModel::receiveData, m_receiver, &TIODeviceReceiver::receiveData, Qt::ConnectionType::QueuedConnection);
        connect(m_receiver, &TIODeviceReceiver::dataReceived, this, &TIODeviceModel::dataReceived, Qt::ConnectionType::QueuedConnection);
        connect(m_receiver, &TIODeviceReceiver::receiveFailed, this, &TIODeviceModel::receiveFailed, Qt::ConnectionType::QueuedConnection);
        connect(this, &TIODeviceModel::startReceiving, m_receiver, &TIODeviceReceiver::startReceiving, Qt::ConnectionType::QueuedConnection);
        connect(this, &TIODeviceModel::stopReceiving, m_receiver, &TIODeviceReceiver::stopReceiving, Qt::ConnectionType::QueuedConnection);

        receiverThread.start();
        senderThread.start();

        emit initialized(this);
    }

    return ok;
}

bool TIODeviceModel::deInit()
{
    if (!isInit()) {
        return false;
    }

    bool ok;
    m_IODevice->deInit(&ok);

    if (ok) {
        m_isInit = false;

        receiverThread.terminate();
        senderThread.terminate();

        emit deinitialized(this);
    }

    return ok;
}

TConfigParam TIODeviceModel::preInitParams() const
{
    return m_IODevice->getPreInitParams();
}

TConfigParam TIODeviceModel::postInitParams() const
{
    return m_IODevice->getPostInitParams();
}

TConfigParam TIODeviceModel::setPreInitParams(const TConfigParam & param)
{
    return m_IODevice->setPreInitParams(param);
}

TConfigParam TIODeviceModel::setPostInitParams(const TConfigParam & param)
{
    return m_IODevice->setPostInitParams(param);
}

int TIODeviceModel::childrenCount() const
{
    return 0;
}

TProjectItem *TIODeviceModel::child(int row) const
{
    return nullptr;
}

QVariant TIODeviceModel::status() const
{
    if (m_isInit) {
        return tr("Initialized");
    }
    else {
        return tr("Uninitialized");
    }
}

void TIODeviceModel::writeData(QByteArray data)
{
    if (!m_sending) {
        m_sending = true;
        emit sendData(data);
    }
    else {
        emit writeBusy();
    }
}

void TIODeviceModel::readData(int length)
{
    if (!m_receiving) {
        m_receiving = true;
        emit receiveData(length);
    }
    else {
        emit readBusy();
    }
}

void TIODeviceModel::enableAutoRead()
{
    if (!m_receiving) {
        m_receiving = true;
        emit startReceiving();
    }
    else {
        emit readBusy();
    }
}

void TIODeviceModel::disableAutoRead()
{
    emit stopReceiving();
    m_receiving = false;
}

void TIODeviceModel::dataSent(QByteArray data)
{
    emit dataWritten(data);
    m_sending = false;
}

void TIODeviceModel::dataReceived(QByteArray data)
{
    emit dataRead(data);
    m_receiving = m_autoReceive;
}

void TIODeviceModel::sendFailed()
{
    emit writeFailed();
    m_sending = false;
}

void TIODeviceModel::receiveFailed()
{
    emit readFailed();
    m_receiving = m_autoReceive;
}

TIODeviceReceiver::TIODeviceReceiver(TIODevice * IODevice, QObject * parent)
    : QObject(parent), m_IODevice(IODevice)
{

}

void TIODeviceReceiver::receiveData(int length)
{
    size_t remainingBytes = (size_t)length;
    quint8 buffer[DATA_BLOCK_SIZE];
    QByteArray data;

    while (remainingBytes > 0) {
        size_t bytesToReceive = (remainingBytes > DATA_BLOCK_SIZE ? DATA_BLOCK_SIZE : remainingBytes);

        size_t bytesReceived = m_IODevice->readData(buffer, bytesToReceive);

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

void TIODeviceReceiver::startReceiving()
{
    m_stopReceiving = false;

    quint8 buffer[DATA_BLOCK_SIZE];

    while (!m_stopReceiving) {
        size_t newBytes = m_IODevice->readData(buffer, DATA_BLOCK_SIZE);

        QByteArray data((char*)buffer, newBytes);

        if (!data.isEmpty()) {
            emit dataReceived(data);
        }

        QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents);
    }
}

void TIODeviceReceiver::stopReceiving()
{
    m_stopReceiving = true;
}

TIODeviceSender::TIODeviceSender(TIODevice * IODevice, QObject * parent)
    : QObject(parent), m_IODevice(IODevice)
{

}

void TIODeviceSender::sendData(QByteArray data)
{
    quint8 * buffer = (quint8*)data.data();

    size_t bytesSent = m_IODevice->readData(buffer, data.length());

    if (bytesSent < (size_t)data.length()) {
        emit sendFailed();
    }

    if (bytesSent > 0) {
        emit dataSent(data.first(bytesSent));
    }
}
