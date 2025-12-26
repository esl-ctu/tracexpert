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

#include "tprojectunit.h"
#include "../scenario/tscenario.h"
#include "../protocol/tprotocol.h"

TProjectUnit * TProjectUnit::instantiate(const QString & typeName) {
    if(typeName == "scenario") return new TScenario();
    if(typeName == "protocol") return new TProtocol();

    qCritical() << "Failed to instatiate TProjectUnit: unknown type: " << typeName;
    return nullptr;
}

TProjectUnit * TProjectUnit::deserialize(const QString & typeName, QDataStream &in) {
    TProjectUnit * unit = instantiate(typeName);

    if (!unit) {
        qWarning() << "Cannot deserialize unknown TProjectUnit type: " << typeName;
        return nullptr;
    }

    unit->deserialize(in);
    return unit;
}
