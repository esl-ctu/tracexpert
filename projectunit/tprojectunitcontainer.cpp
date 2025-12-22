#include "tprojectunitcontainer.h"

#include "tprojectunit.h"
#include "tprojectunitmodel.h"
#include "../project/tprojectmodel.h"

#include <QFileDialog>
#include <QMessageBox>


TProjectUnitContainer::TProjectUnitContainer(TProjectModel * parent)
    : TProjectItem(parent->model(), parent)
{}

TProjectUnitContainer::~TProjectUnitContainer() {
    qDeleteAll(m_unitModels);
    m_unitModels.clear();
}

void TProjectUnitContainer::showManager() {
    emit showManagerRequested();
}

const QString & TProjectUnitContainer::unitTypeName() const {
    return m_unitTypeName;
}

TProjectUnitModel * TProjectUnitContainer::getByName(const QString &name, bool *ok) const {
    for(TProjectUnitModel * model : m_unitModels) {
        if(model->name() == name) {
            if(ok != nullptr) *ok = true;
            return model;
        }
    }

    if(ok != nullptr) *ok = false;
    return nullptr;
}

int TProjectUnitContainer::getIndexByName(const QString &name, bool *ok) const {
    int count = m_unitModels.size();
    for(int i = 0; i < count; i++) {
        if(m_unitModels[i]->name() == name) {
            if(ok != nullptr) *ok = true;
            return i;
        }
    }

    if(ok != nullptr) *ok = false;
    return -1;
}

TProjectUnitModel * TProjectUnitContainer::at(int index) {
    return m_unitModels[index];
}

int TProjectUnitContainer::count() const{
    return m_unitModels.size();
}

bool TProjectUnitContainer::add(TProjectUnitModel * model) {
    if(model == nullptr) {
        return false;
    }

    TProjectUnitModel * moduleWithDesiredName = getByName(model->name());
    if(moduleWithDesiredName != nullptr) {
        return false;
    }

    connect(model, &TProjectUnitModel::editorRequested, this, &TProjectUnitContainer::editorRequested);

    int index = m_unitModels.size();

    beginInsertChild(index);
    beginInsertRows(QModelIndex(), index, index);
    m_unitModels.insert(index, model);
    endInsertRows();
    endInsertChild();

    sort();
    return true;
}

bool TProjectUnitContainer::add(TProjectUnit * unit) {
    if(unit == nullptr) {
        return false;
    }

    TProjectUnitModel * moduleWithDesiredName = getByName(unit->name());
    if(moduleWithDesiredName != nullptr) {
        return false;
    }

    TProjectUnitModel * model = TProjectUnitModel::instantiate(m_unitTypeName, this, unit);
    this->add(model);

    return true;
}

bool TProjectUnitContainer::update(int index, TProjectUnit * unit) {
    if(unit == nullptr) {
        return false;
    }

    if(index >= 0 && index <= m_unitModels.size()) {
        m_unitModels[index]->setUnit(unit);
        emit dataChanged(QAbstractTableModel::index(index, 0), QAbstractTableModel::index(index, this->columnCount()));

        sort();
        return true;
    }

    return false;
}

bool TProjectUnitContainer::remove(int index) {
    if(index >= 0 && index < m_unitModels.size()) {
        beginRemoveChild(index);
        beginRemoveRows(QModelIndex(), index, index);
        delete m_unitModels[index];
        m_unitModels.removeAt(index);
        endRemoveRows();
        endRemoveChild();
        return true;
    }

    return false;
}

void TProjectUnitContainer::sort() {
    int i, j, n = m_unitModels.size();
    bool swapped;
    for (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (m_unitModels[j]->name().compare(m_unitModels[j + 1]->name(), Qt::CaseInsensitive) > 0) {
                m_unitModels.swapItemsAt(j, j + 1);
                emit dataChanged(QAbstractTableModel::index(j, 0), QAbstractTableModel::index(j, this->columnCount()));
                emit dataChanged(QAbstractTableModel::index(j + 1, 0), QAbstractTableModel::index(j + 1, this->columnCount()));
                swapped = true;
            }
        }

        if (swapped == false)
            break;
    }

    itemDataChanged();
    emit itemsUpdated();
}

int TProjectUnitContainer::rowCount(const QModelIndex &parent) const {
    return count();
}

int TProjectUnitContainer::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant TProjectUnitContainer::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_unitModels.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0:
                return m_unitModels[index.row()]->unit()->name();
            case 1:
                return m_unitModels[index.row()]->unit()->description();
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QVariant TProjectUnitContainer::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        switch(section) {
            case 0:
                return QStringLiteral("Name");
            case 1:
                return QStringLiteral("Description");
            default:
                return QVariant();
        }
    }

    return QVariant();
}

int TProjectUnitContainer::childrenCount() const
{
    return m_unitModels.size();
}

TProjectItem * TProjectUnitContainer::child(int row) const
{
    return m_unitModels[row];
}

TProjectItem::Status TProjectUnitContainer::status() const
{
    return Status::None;
}
