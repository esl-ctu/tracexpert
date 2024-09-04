#include "tiodevicemodel.h"

#include <QCoreApplication>
#include <QVariant>

#include "tiodevicecontainer.h"
#include "tcomponentmodel.h"

TIODeviceModel::TIODeviceModel(TIODevice * IODevice, TIODeviceContainer * parent, bool manual)
    : TProjectItem(parent->model(), parent), TDeviceModel(IODevice, parent, manual), m_IODevice(IODevice)
{
    m_typeName = "iodevice";

    m_sending = false;
    m_receiving = false;
    m_autoReceive = false;
}

void TIODeviceModel::show()
{
    emit showRequested(this);
}

bool TIODeviceModel::init()
{
    if (isInit() || !TPluginUnitModel::init()) {
        return false;
    }

    m_isInit = true;

    if (senderThread.isRunning())
        senderThread.terminate();

    if (receiverThread.isRunning())
        receiverThread.terminate();

    m_sender = new TIODeviceSender(m_IODevice);
    m_sender->moveToThread(&senderThread);

    m_receiver = new TIODeviceReceiver(m_IODevice);
    m_receiver->moveToThread(&receiverThread);

    connect(this, &TIODeviceModel::sendData, m_sender, &TIODeviceSender::sendData, Qt::ConnectionType::QueuedConnection);
    connect(m_sender, &TIODeviceSender::dataSent, this, &TIODeviceModel::dataSent, Qt::ConnectionType::QueuedConnection);
    connect(m_sender, &TIODeviceSender::sendFailed, this, &TIODeviceModel::sendFailed, Qt::ConnectionType::QueuedConnection);
    connect(&senderThread, &QThread::finished, m_sender, &QObject::deleteLater);

    connect(this, &TIODeviceModel::receiveData, m_receiver, &TIODeviceReceiver::receiveData, Qt::ConnectionType::QueuedConnection);
    connect(m_receiver, &TIODeviceReceiver::dataReceived, this, &TIODeviceModel::dataReceived, Qt::ConnectionType::QueuedConnection);
    connect(m_receiver, &TIODeviceReceiver::receiveFailed, this, &TIODeviceModel::receiveFailed, Qt::ConnectionType::QueuedConnection);
    connect(this, &TIODeviceModel::startReceiving, m_receiver, &TIODeviceReceiver::startReceiving, Qt::ConnectionType::QueuedConnection);
    connect(this, &TIODeviceModel::stopReceiving, m_receiver, &TIODeviceReceiver::stopReceiving, Qt::ConnectionType::QueuedConnection);
    connect(&receiverThread, &QThread::finished, m_receiver, &QObject::deleteLater);

    receiverThread.start();
    senderThread.start();

    emit initialized(this);

    return true;
}

bool TIODeviceModel::deInit()
{
    if (!isInit() || !TPluginUnitModel::deInit()) {
        return false;
    }

    m_isInit = false;

    receiverThread.quit();
    senderThread.quit();

    emit deinitialized(this);

    return true;
}

bool TIODeviceModel::remove()
{
    TComponentModel * component = dynamic_cast<TComponentModel *>(TProjectItem::parent()->parent());
    if (!component)
        return false;

    return component->removeIODevice(this);
}

void TIODeviceModel::bind(TCommon * unit)
{
    m_IODevice = static_cast<TIODevice *>(unit);
    TPluginUnitModel::bind(m_IODevice);
}

void TIODeviceModel::release()
{
    m_IODevice = nullptr;
    TPluginUnitModel::release();
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
    : TReceiver(parent), m_IODevice(IODevice)
{

}

size_t TIODeviceReceiver::readData(uint8_t * buffer, size_t len)
{
    return m_IODevice->readData(buffer, len);
}

TIODeviceSender::TIODeviceSender(TIODevice * IODevice, QObject * parent)
    : TSender(parent), m_IODevice(IODevice)
{

}

size_t TIODeviceSender::writeData(const uint8_t * buffer, size_t len)
{
    return m_IODevice->writeData(buffer, len);
}
