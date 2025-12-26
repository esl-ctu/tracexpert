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

#ifndef TSCENARIOCONTAINER_H
#define TSCENARIOCONTAINER_H

#include <QAbstractTableModel>
#include "../projectunit/tprojectunitcontainer.h"

/*!
 * \brief The TScenarioContainer class represents a container for Scenarios.
 *
 * The class represents a container for TScenarioModel objects.
 * It is a model for the Scenarios view in the Project view.
 * It is also a model for the Scenarios table view in the Scenario Manager.
 */
class TScenarioContainer : public TProjectUnitContainer {

public:
    explicit TScenarioContainer(TProjectModel * parent);
    QString name() const override;
};

#endif // TSCENARIOCONTAINER_H
