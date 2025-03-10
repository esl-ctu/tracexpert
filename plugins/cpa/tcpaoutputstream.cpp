#include "tcpaoutputstream.h"

TCPAOutputStream::TCPAOutputStream(QString name, QString info, std::function<size_t(const uint8_t *, size_t)> writeData)
    : m_name(name), m_info(info), m_writeData(writeData)
{

}

QString TCPAOutputStream::getName() const
{
    return m_name;
}

QString TCPAOutputStream::getInfo() const
{
    return m_info;
}

size_t TCPAOutputStream::writeData(const uint8_t * buffer, size_t len)
{
    return m_writeData(buffer, len);
}

