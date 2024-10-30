#include "tiodevicereceiver.h"

TIODeviceReceiver::TIODeviceReceiver(TIODevice * IODevice, QObject * parent)
    : TReceiver(parent), m_IODevice(IODevice)
{

}

size_t TIODeviceReceiver::readData(uint8_t * buffer, size_t len)
{
    return m_IODevice->readData(buffer, len);
}
