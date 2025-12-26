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

#ifndef TSCENARIOCONFIGWIDGET_H
#define TSCENARIOCONFIGWIDGET_H

#include "widgets/tconfigparamwidget.h"

class TScenarioConfigParamWidget : public TConfigParamWidget
{
    Q_OBJECT

public:
    explicit TScenarioConfigParamWidget(const TConfigParam & param, QWidget * parent = nullptr, bool readOnly = false);

    void setDynamicParamNames(QStringList allowedDynamicParamNames, QStringList selectedDynamicParamNames);
    QStringList getSelectedDynamicParamNames() const;

    void setParam(const TConfigParam & param);

private:
    void addCheckBox(const TConfigParam & param, QTreeWidgetItem * parent, bool forceAllowed = false);

    QStringList m_allowedDynamicParamNames;
    QStringList m_selectedDynamicParamNames;
};

#endif // TSCENARIOCONFIGWIDGET_H
