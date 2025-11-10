#include "tcipherinputstream.h"

TCipherInputStream::TCipherInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData, std::function<size_t(void)> availableBytes)
    : m_name(name), m_info(info), m_readData(readData), m_availableBytes(availableBytes)
{

}

QString TCipherInputStream::getName() const
{
    return m_name;
}

QString TCipherInputStream::getInfo() const
{
    return m_info;
}

size_t TCipherInputStream::readData(uint8_t * buffer, size_t len)
{
    return m_readData(buffer, len);
}

size_t TCipherInputStream::availableBytes()
{
    return m_availableBytes();
}

