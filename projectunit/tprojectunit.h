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
// Adam Švehla (initial author)

#ifndef TPROJECTUNIT_H
#define TPROJECTUNIT_H

#include <QString>

class TProjectUnit
{
public:
    virtual ~TProjectUnit() {};

    virtual const QString & name() const = 0;
    virtual const QString & description() const = 0;

    virtual void setName(const QString & name) = 0;
    virtual void setDescription(const QString & name) = 0;

    virtual TProjectUnit * copy() const = 0;

    virtual void serialize(QDataStream &out) const = 0;
    virtual void deserialize(QDataStream &in) = 0;

    static TProjectUnit * instantiate(const QString & typeName);
    static TProjectUnit * deserialize(const QString & typeName, QDataStream &in);
};

#endif // TPROJECTUNIT_H
