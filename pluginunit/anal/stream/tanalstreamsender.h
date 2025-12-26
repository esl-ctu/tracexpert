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

#ifndef TANALSTREAMSENDER_H
#define TANALSTREAMSENDER_H

#include "../../common/sender/tsender.h"

#include "tanaldevice.h"

class TAnalStreamSender : public TSender
{
    Q_OBJECT

public:
    explicit TAnalStreamSender(TAnalOutputStream * stream, QObject * parent = nullptr);

    QString name() override;
    QString info() override;

protected:
    size_t writeData(const uint8_t * buffer, size_t len) override;

private:
    TAnalOutputStream * m_stream;
};

#endif // TANALSTREAMSENDER_H
