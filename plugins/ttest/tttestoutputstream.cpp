#include "tttestoutputstream.h"

TTTestOutputStream::TTTestOutputStream(QString name, QString info, std::function<size_t(const uint8_t *, size_t)> writeData)
    : m_name(name), m_info(info), m_writeData(writeData)
{

}

QString TTTestOutputStream::getName() const
{
    return m_name;
}

QString TTTestOutputStream::getInfo() const
{
    return m_info;
}

size_t TTTestOutputStream::writeData(const uint8_t * buffer, size_t len)
{
    return m_writeData(buffer, len);
}
