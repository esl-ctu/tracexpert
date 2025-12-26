// COPYRIGHT HEADER BEGIN
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
// COPYRIGHT HEADER END

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
