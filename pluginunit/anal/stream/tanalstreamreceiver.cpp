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
// Vojtěch Miškovský (initial author)
// Adam Švehla
// Petr Socha

#include "tanalstreamreceiver.h"

TAnalStreamReceiver::TAnalStreamReceiver(TAnalInputStream * stream, QObject * parent)
    : TReceiver(parent), m_stream(stream)
{

}

QString TAnalStreamReceiver::name()
{
    return m_stream->getName();
}

QString TAnalStreamReceiver::info()
{
    return m_stream->getInfo();
}

size_t TAnalStreamReceiver::readData(uint8_t * buffer, size_t len)
{
    return m_stream->readData(buffer, len);
}

std::optional<size_t> TAnalStreamReceiver::availableBytes() {
    return m_stream->availableBytes();
}
