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

#ifndef TSCOPECONTAINER_H
#define TSCOPECONTAINER_H

#include "../tpluginunitcontainer.h"
#include "../scope/tscopemodel.h"

class TComponentModel;

class TScopeContainer : public TPluginUnitContainer
{
    Q_OBJECT

public:
    explicit TScopeContainer(TComponentModel * parent);
    
    int count() const override;
    TScopeModel * at(int index) const override;
    TScopeModel * getByName(const QString &name) const override;

    void add(TScopeModel * unit);
    bool remove(TScopeModel * unit);

    TScopeModel * hasName(QString name) const;
    
    QString name() const override;

private:
    QList<TScopeModel *> m_scopes;
};

#endif // TSCOPECONTAINER_H
