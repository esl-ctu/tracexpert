#include "tscenario.h"
#include "tscenariocontainer.h"
#include "../project/tprojectmodel.h"

TScenarioContainer::TScenarioContainer(TProjectModel * parent) : TProjectItem(parent->model(), parent) {
    m_typeName = "scenarios";
}

TScenarioContainer::~TScenarioContainer() {
    qDeleteAll(m_scenarioModels);
    m_scenarioModels.clear();
}

TScenarioModel * TScenarioContainer::getByName(const QString &name, bool *ok) const {
    for(TScenarioModel * scenarioModel : m_scenarioModels) {
        if(scenarioModel->name() == name) {
            if(ok != nullptr) *ok = true;
            return scenarioModel;
        }
    }

    if(ok != nullptr) *ok = false;
    return nullptr;
}

int TScenarioContainer::getIndexByName(const QString &name, bool *ok) const {
    int count = m_scenarioModels.size();
    for(int i = 0; i < count; i++) {
        if(m_scenarioModels[i]->name() == name) {
            if(ok != nullptr) *ok = true;
            return i;
        }
    }

    if(ok != nullptr) *ok = false;
    return -1;
}

TScenarioModel * TScenarioContainer::at(int index) {
    return m_scenarioModels[index];
}

int TScenarioContainer::count() const{
    return m_scenarioModels.size();
}

bool TScenarioContainer::add(TScenarioModel * scenarioModel) {
    if(scenarioModel == nullptr) {
        return false;
    }

    TScenarioModel * moduleWithDesiredName = getByName(scenarioModel->name());
    if(moduleWithDesiredName != nullptr) {
        return false;
    }

    int index = m_scenarioModels.size();

    beginInsertChild(index);
    beginInsertRows(QModelIndex(), index, index);
    m_scenarioModels.insert(index, scenarioModel);
    endInsertRows();
    endInsertChild();

    sort();
    return true;
}

bool TScenarioContainer::add(TScenario * scenario) {
    if(scenario == nullptr) {
        return false;
    }

    TScenarioModel * scenarioModel = new TScenarioModel(scenario, this);

    if(!this->add(scenarioModel)) {
        delete scenarioModel;
        return false;
    }

    return true;
}

bool TScenarioContainer::update(int index, TScenario * scenario) {
    if(scenario == nullptr) {
        return false;
    }

    if(index >= 0 && index <= m_scenarioModels.size()) {
        m_scenarioModels[index]->setScenario(scenario);
        emit dataChanged(QAbstractTableModel::index(index, 0), QAbstractTableModel::index(index, this->columnCount()));

        sort();
        return true;
    }

    return false;
}

bool TScenarioContainer::remove(int index) {
    if(index >= 0 && index < m_scenarioModels.size()) {
        beginRemoveChild(index);
        beginRemoveRows(QModelIndex(), index, index);
        delete m_scenarioModels[index];
        m_scenarioModels.removeAt(index);
        endRemoveRows();
        endRemoveChild();
        return true;
    }

    return false;
}


void TScenarioContainer::sort() {
    int i, j, n = m_scenarioModels.size();
    bool swapped;
    for (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (m_scenarioModels[j]->name().compare(m_scenarioModels[j + 1]->name(), Qt::CaseInsensitive) > 0) {
                m_scenarioModels.swapItemsAt(j, j + 1);
                emit dataChanged(QAbstractTableModel::index(j, 0), QAbstractTableModel::index(j, this->columnCount()));
                emit dataChanged(QAbstractTableModel::index(j + 1, 0), QAbstractTableModel::index(j + 1, this->columnCount()));
                swapped = true;
            }
        }

        if (swapped == false)
            break;
    }

    itemDataChanged();
    emit scenariosUpdated();
}


int TScenarioContainer::rowCount(const QModelIndex &parent) const {
    return count();
}

int TScenarioContainer::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant TScenarioContainer::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_scenarioModels.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case 0:
                return ((TScenarioModel *)m_scenarioModels[index.row()])->scenario()->getName();
            case 1:
                return ((TScenarioModel *)m_scenarioModels[index.row()])->scenario()->getDescription();
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QVariant TScenarioContainer::headerData(int section, Qt::Orientation orientation, int role) const {
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


int TScenarioContainer::childrenCount() const
{
    return m_scenarioModels.size();
}

TProjectItem * TScenarioContainer::child(int row) const
{
    return m_scenarioModels[row];
}

QString TScenarioContainer::name() const
{
    return tr("Scenarios");
}

TProjectItem::Status TScenarioContainer::status() const
{
    return Status::None;
}
