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

#include "tprojectitem.h"

#include <QModelIndex>
#include <QApplication>
#include <QStyle>

#include "tprojectmodel.h"

TProjectItem::TProjectItem(TProjectModel * model, TProjectItem * parent)
    : m_model(model), m_parent(parent)
{

}

TProjectItem::~TProjectItem()
{

}

QVariant TProjectItem::statusText(Status status)
{
    switch (status) {
        case None:
            return QObject::tr("");
        case Unavailable:
            return QObject::tr("Unavailable");
        case Uninitialized:
            return QObject::tr("Uninitialized");
        case Initialized:
            return QObject::tr("Initialized");
    }

    return QVariant();
}

QVariant TProjectItem::statusIcon(Status status)
{
    switch (status) {
        case None:
            return QVariant();
        case Unavailable:
            return QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton);
        case Uninitialized:
            return QApplication::style()->standardIcon(QStyle::SP_DialogNoButton);
        case Initialized:
            return QApplication::style()->standardIcon(QStyle::SP_DialogYesButton);
    }

    return QVariant();
}

int TProjectItem::row() const
{
    if (!m_parent) {
        return 0;
    }

    for (int i = 0; i < m_parent->childrenCount(); i++) {
        if (m_parent->child(i) == this) {
            return i;
        }
    }

    return -1;
}

TProjectItem * TProjectItem::parent()
{
    return m_parent;
}

TProjectModel * TProjectItem::model()
{
    return m_model;
}

QString TProjectItem::typeName() const
{
    return m_typeName;
}

bool TProjectItem::toBeSaved() const
{
    return true;
}

QDomElement TProjectItem::save(QDomDocument & document) const
{
    QDomElement element = document.createElement(typeName());

    for (int i = 0; i < childrenCount(); i++) {
        if (child(i)->toBeSaved())
            element.appendChild(child(i)->save(document));
    }

    return element;
}

void TProjectItem::beginInsertChild(int childRow)
{
    QModelIndex index = m_model->createIndex(row(), 0, this);

    m_model->beginInsertRows(index, childRow, childRow);
}

void TProjectItem::endInsertChild()
{
    m_model->endInsertRows();
}

void TProjectItem::beginRemoveChild(int childRow)
{
    QModelIndex index = m_model->createIndex(row(), 0, this);

    m_model->beginRemoveRows(index, childRow, childRow);
}

void TProjectItem::endRemoveChild()
{
    m_model->endRemoveRows();
}

void TProjectItem::itemDataChanged()
{
    QModelIndex index = m_model->createIndex(row(), 0, m_parent);

    m_model->emitDataChanged(index, index);
}
