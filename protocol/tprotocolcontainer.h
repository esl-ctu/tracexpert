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

#ifndef TPROTOCOLCONTAINER_H
#define TPROTOCOLCONTAINER_H

#include <QAbstractTableModel>
#include "../projectunit/tprojectunitcontainer.h"

/*!
 * \brief The TProtocolContainer class represents a container for Protocols.
 *
 * The class represents a container for TProtocolModel objects.
 * It is a model for the Protocols view in the Project view.
 * It is also a model for the Protocols table view in the Protocol Manager.
 */
class TProtocolContainer : public TProjectUnitContainer {

public:
    explicit TProtocolContainer(TProjectModel * parent);
    QString name() const override;
};

#endif // TPROTOCOLCONTAINER_H
