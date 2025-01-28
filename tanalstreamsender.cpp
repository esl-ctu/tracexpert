#include "tanalstreamsender.h"

TAnalStreamSender::TAnalStreamSender(TAnalOutputStream * stream, QObject * parent)
    : TSender(parent), m_stream(stream)
{

}

QString TAnalStreamSender::name()
{
    return m_stream->getName();
}

QString TAnalStreamSender::info()
{
    return m_stream->getInfo();
}

size_t TAnalStreamSender::writeData(const uint8_t * buffer, size_t len)
{
    return m_stream->writeData(buffer, len);
}
