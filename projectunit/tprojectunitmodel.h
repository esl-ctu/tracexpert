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

#ifndef TPROJECTUNITMODEL_H
#define TPROJECTUNITMODEL_H

#include "../project/tprojectitem.h"
#include "tprojectunitcontainer.h"
#include <QObject>

class TProjectUnitModel : public QObject, public TProjectItem {
    Q_OBJECT

public:
    ~TProjectUnitModel();

    void openEditor();

    TProjectUnit * unit() const;
    void setUnit(TProjectUnit * unit);

    // methods for TProjectItem - to be able to show Scenarios in the Project view
    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    Status status() const override;

    bool toBeSaved() const override;
    QDomElement save(QDomDocument & document) const override;
    void load(QDomElement * element);

    static TProjectUnitModel * instantiate(const QString & typeName, TProjectUnitContainer * parent, TProjectUnit * unit = nullptr);

signals:
    void editorRequested(TProjectUnitModel * projectUnitModel);

protected:
    TProjectUnitModel(const QString & typeName, TProjectUnitContainer * parent, TProjectUnit * item = nullptr);

    TProjectUnit * m_unit;
};

#endif // TPROJECTUNITMODEL_H
