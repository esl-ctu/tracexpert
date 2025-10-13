#include "tcipheroutputstream.h"

TCipherOutputStream::TCipherOutputStream(QString name, QString info, std::function<size_t(const uint8_t *, size_t)> writeData)
    : m_name(name), m_info(info), m_writeData(writeData)
{

}

QString TCipherOutputStream::getName() const
{
    return m_name;
}

QString TCipherOutputStream::getInfo() const
{
    return m_info;
}

size_t TCipherOutputStream::writeData(const uint8_t * buffer, size_t len)
{
    return m_writeData(buffer, len);
}

