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
