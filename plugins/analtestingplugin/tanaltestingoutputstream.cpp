#include "tanaltestingoutputstream.h"

TAnalTestingOutputStream::TAnalTestingOutputStream(QString name, QString info, std::function<size_t(const uint8_t *, size_t)> writeData)
    : m_name(name), m_info(info), m_writeData(writeData)
{

}

QString TAnalTestingOutputStream::getName() const
{
    return m_name;
}

QString TAnalTestingOutputStream::getInfo() const
{
    return m_info;
}

size_t TAnalTestingOutputStream::writeData(const uint8_t * buffer, size_t len)
{
    return m_writeData(buffer, len);
}
