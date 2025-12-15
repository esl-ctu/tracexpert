#include "tpredictinputstream.h"

TPredictInputStream::TPredictInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData, std::function<size_t(void)> availableBytes)
    : m_name(name), m_info(info), m_readData(readData), m_availableBytes(availableBytes)
{

}

QString TPredictInputStream::getName() const
{
    return m_name;
}

QString TPredictInputStream::getInfo() const
{
    return m_info;
}

size_t TPredictInputStream::readData(uint8_t * buffer, size_t len)
{
    return m_readData(buffer, len);
}

std::optional<size_t> TPredictInputStream::availableBytes()
{
    return m_availableBytes();
}
