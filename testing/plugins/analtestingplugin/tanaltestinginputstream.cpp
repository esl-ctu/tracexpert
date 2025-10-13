#include "tanaltestinginputstream.h"

TAnalTestingInputStream::TAnalTestingInputStream(QString name, QString info, std::function<size_t(uint8_t *, size_t)> readData)
    : m_name(name), m_info(info), m_readData(readData)
{

}

QString TAnalTestingInputStream::getName() const
{
    return m_name;
}

QString TAnalTestingInputStream::getInfo() const
{
    return m_info;
}

size_t TAnalTestingInputStream::readData(uint8_t * buffer, size_t len)
{
    return m_readData(buffer, len);
}
