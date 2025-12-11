#include "tttestinputstream.h"

TTTestInputStream::TTTestInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData, std::function<size_t(void)> availableBytes)
    : m_name(name), m_info(info), m_readData(readData), m_availableBytes(availableBytes)
{

}

QString TTTestInputStream::getName() const
{
    return m_name;
}

QString TTTestInputStream::getInfo() const
{
    return m_info;
}

size_t TTTestInputStream::readData(uint8_t * buffer, size_t len)
{
    return m_readData(buffer, len);
}

std::optional<size_t> TTTestInputStream::availableBytes()
{
    return m_availableBytes();
}
