#include "tiodevicesender.h"

TIODeviceSender::TIODeviceSender(TIODevice * IODevice, QObject * parent)
    : TSender(parent), m_IODevice(IODevice)
{

}

size_t TIODeviceSender::writeData(const uint8_t * buffer, size_t len)
{
    return m_IODevice->writeData(buffer, len);
}
