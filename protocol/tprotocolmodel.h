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
// Vojtěch Miškovský

#ifndef TPROTOCOLMODEL_H
#define TPROTOCOLMODEL_H

#include "../projectunit/tprojectunitmodel.h"

class TProtocol;
class TProtocolContainer;

/*!
 * \brief The TProtocolModel class represents a model for a Protocol.
 *
 * The class represents a model for a Protocol.
 * It is a model for the Protocol view in the Project view.
 * It is also a model for the Protocol table view in the Protocol Manager.
 *
 */
class TProtocolModel : public TProjectUnitModel {

public:
    TProtocolModel(TProtocolContainer * parent, TProtocol * protocol = nullptr);

    TProtocol * protocol() const;
};

#endif // TPROTOCOLMODEL_H
