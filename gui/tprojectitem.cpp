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
