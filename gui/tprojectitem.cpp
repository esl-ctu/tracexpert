#include "tprojectitem.h"

#include <QModelIndex>

#include "tprojectmodel.h"

TProjectItem::TProjectItem(TProjectModel * model, TProjectItem * parent)
    : m_model(model), m_parent(parent)
{

}

TProjectItem::~TProjectItem()
{

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
