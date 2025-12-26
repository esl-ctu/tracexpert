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

#ifndef TPROJECTITEM_H
#define TPROJECTITEM_H

#include <QList>
#include <QtXml/QDomDocument>

class TProjectModel;

class TProjectItem
{
public:
    explicit TProjectItem(TProjectModel * model, TProjectItem * parent);
    virtual ~TProjectItem();

    enum Status { None, Unavailable, Uninitialized, Initialized };

    static QVariant statusText(Status status);
    static QVariant statusIcon(Status status);

    virtual int childrenCount() const = 0;
    virtual TProjectItem * child(int row) const = 0;
    virtual QString name() const = 0;
    virtual Status status() const = 0;

    int row() const;

    TProjectItem * parent();
    TProjectModel * model();

    QString typeName() const;

    virtual bool toBeSaved() const;
    virtual QDomElement save(QDomDocument & document) const;

protected:
    void beginInsertChild(int childRow);
    void endInsertChild();
    void beginRemoveChild(int childRow);
    void endRemoveChild();
    void itemDataChanged();

    QString m_typeName = "unknown";

protected:
    TProjectModel * m_model;
    TProjectItem * m_parent;
};

#endif // TPROJECTITEM_H
