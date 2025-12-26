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

#ifndef TABSTRACTLISTMODEL_H
#define TABSTRACTLISTMODEL_H

#include <QAbstractListModel>

template <class T>
class TAbstractTableModel : public virtual QAbstractTableModel {

public:
    TAbstractTableModel(QObject * parent = nullptr) : QAbstractTableModel(parent), m_items() { }
    TAbstractTableModel(QList<T> & initialItems, QObject * parent = nullptr) : QAbstractTableModel(parent), m_items(initialItems) { }

    /*
     * need to be implemented by child classes:
     *
        virtual int columnCount(const QModelIndex &parent = QModelIndex());
        virtual QVariant data(const QModelIndex &index, int role);
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole);
    */

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        return m_items.size();
    }

    T getItem(int index) const {
        return m_items[index];
    }

    const QList<T> & getItems() const {
        return m_items;
    }

    void addItem(const T & item) {
        int index = m_items.size();

        beginInsertRows(QModelIndex(), index, index);
        m_items.insert(index, item);
        endInsertRows();
    }

    void addItem(int index, const T & item) {
        if(index < 0 || index > m_items.size()) {
            index = m_items.size();
        }

        beginInsertRows(QModelIndex(), index, index);
        m_items.insert(index, item);
        endInsertRows();
    }

    bool updateItem(int index, const T & item) {
        if(index >= 0 && index <= m_items.size()) {
            m_items[index] = item;
            emit dataChanged(QAbstractTableModel::index(index, 0), QAbstractTableModel::index(index, this->columnCount()));
            return true;
        }

        return false;
    }

    bool removeItem(int index) {
        if(index >= 0 && index < m_items.size()) {
            beginRemoveRows(QModelIndex(), index, index);
            m_items.removeAt(index);
            endRemoveRows();
            return true;
        }

        return false;
    }

    bool swapItems(int index1, int index2) {
        if(index1 >= 0 && index1 < m_items.size() && index2 >= 0 && index2 < m_items.size()) {
            m_items.swapItemsAt(index1, index2);
            emit dataChanged(QAbstractTableModel::index(index1, 0), QAbstractTableModel::index(index1, this->columnCount()));
            emit dataChanged(QAbstractTableModel::index(index2, 0), QAbstractTableModel::index(index2, this->columnCount()));
            return true;
        }
        return false;
    }

    using QAbstractTableModel::sort;
    void sort() {
        int i, j, n = m_items.size();
        bool swapped;
        for (i = 0; i < n - 1; i++) {
            swapped = false;
            for (j = 0; j < n - i - 1; j++) {
                if (m_items[j].getName().compare(m_items[j + 1].getName(), Qt::CaseInsensitive) > 0) {
                    swapItems(j, j + 1);
                    swapped = true;
                }
            }

            if (swapped == false)
                break;
        }
    }

protected:
    QList<T> m_items;
};

#endif // TABSTRACTLISTMODEL_H
