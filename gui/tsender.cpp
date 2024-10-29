#include "tsender.h"

TSender::TSender(QObject *parent)
    : QObject(parent)
{

}

void TSender::sendData(QByteArray data)
{
    quint8 * buffer = (quint8*)data.data();

    size_t bytesSent = writeData(buffer, data.length());

    if (bytesSent < (size_t)data.length()) {
        emit sendFailed();
    }

    if (bytesSent > 0) {
        emit dataSent(data.first(bytesSent));
    }
}
