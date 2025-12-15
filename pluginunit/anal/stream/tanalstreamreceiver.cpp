#include "tanalstreamreceiver.h"

TAnalStreamReceiver::TAnalStreamReceiver(TAnalInputStream * stream, QObject * parent)
    : TReceiver(parent), m_stream(stream)
{

}

QString TAnalStreamReceiver::name()
{
    return m_stream->getName();
}

QString TAnalStreamReceiver::info()
{
    return m_stream->getInfo();
}

size_t TAnalStreamReceiver::readData(uint8_t * buffer, size_t len)
{
    return m_stream->readData(buffer, len);
}

std::optional<size_t> TAnalStreamReceiver::availableBytes() {
    return m_stream->availableBytes();
}
