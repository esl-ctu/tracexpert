// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Petr Socha (initial author)

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

std::optional<size_t> TCipherInputStream::availableBytes()
{
    return m_availableBytes();
}

