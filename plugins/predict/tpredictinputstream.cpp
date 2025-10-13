#include "tpredictinputstream.h"

TPredictInputStream::TPredictInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData)
    : m_name(name), m_info(info), m_readData(readData)
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

