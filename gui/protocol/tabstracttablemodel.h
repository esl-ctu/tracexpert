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

    T getItem(int index) {
        return m_items[index];
    }

    const QList<T> & getItems() const {
        return m_items;
    }

    bool insertItem(int index, const T & item) {
        if(index < 0) {
            index = m_items.size();
        }

        if(index >= 0 && index <= m_items.size()) {
            beginInsertRows(QModelIndex(), index, index);
            m_items.insert(index, item);
            endInsertRows();

            return true;
        }

        return false;
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

protected:
    QList<T> m_items;
};

template <class T>
class TAbstractTableModel<T*> : public virtual QAbstractTableModel {

public:
    TAbstractTableModel(QObject * parent = nullptr) : QAbstractTableModel(parent), m_items() { }
    TAbstractTableModel(QList<T *> & initialItems, QObject * parent = nullptr) : QAbstractTableModel(parent), m_items(initialItems) { }

    ~TAbstractTableModel() {
        qDeleteAll(m_items);
    }

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

    T * getItem(int index) {
        return m_items[index];
    }

    const QList<T*> & getItems() const {
        return m_items;
    }

    bool insertItem(int index, T * item) {
        if(index < 0) {
            index = m_items.size();
        }

        if(index >= 0 && index <= m_items.size()) {
            beginInsertRows(QModelIndex(), index, index);
            m_items.insert(index, item);
            endInsertRows();

            return true;
        }

        return false;
    }

    bool updateItem(int index) {
        if(index >= 0 && index <= m_items.size()) {
            emit dataChanged(QAbstractTableModel::index(index, 0), QAbstractTableModel::index(index, this->columnCount()));
            return true;
        }

        return false;
    }

    bool updateItem(int index, T * item) {
        if(index >= 0 && index <= m_items.size()) {
            delete m_items[index];
            m_items[index] = item;
            emit dataChanged(QAbstractTableModel::index(index, 0), QAbstractTableModel::index(index, this->columnCount()));
            return true;
        }

        return false;
    }

    bool removeItem(int index) {
        if(index >= 0 && index < m_items.size()) {
            beginRemoveRows(QModelIndex(), index, index);
            delete m_items[index];
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

protected:
    QList<T *> m_items;
};

#endif // TABSTRACTLISTMODEL_H
