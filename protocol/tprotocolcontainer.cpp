#include "tprotocolcontainer.h"
#include "tprotocolmodel.h"
#include "../project/tprojectmodel.h"
#include <qfiledialog.h>
#include <QMessageBox>

TProtocolContainer::TProtocolContainer(TProjectModel * parent) : QAbstractTableModel(parent), TProjectItem(parent->model(), parent) {
    m_typeName = "protocols";
}

TProtocolContainer::~TProtocolContainer() {
    qDeleteAll(m_protocolModels);
}

TProtocol TProtocolContainer::getByName(const QString &name, bool *ok) const {
    for(TProtocolModel * protocolModel : m_protocolModels) {
        if(protocolModel->name() == name) {
            if(ok != nullptr) *ok = true;
            return protocolModel->protocol();
        }
    }

    if(ok != nullptr) *ok = false;
    return TProtocol();
}

int TProtocolContainer::getIndexByName(const QString &name, bool *ok) const {
    int count = m_protocolModels.size();
    for(int i = 0; i < count; i++) {
        if(m_protocolModels[i]->name() == name) {
            if(ok != nullptr) *ok = true;
            return i;
        }
    }

    if(ok != nullptr) *ok = false;
    return -1;
}

const TProtocolModel * TProtocolContainer::at(int index) const {
    return m_protocolModels.at(index);
}

int TProtocolContainer::count() const{
    return m_protocolModels.size();
}

bool TProtocolContainer::add(TProtocolModel * protocolModel) {
    bool nameFound;
    getByName(protocolModel->name(), &nameFound);

    if(nameFound) {
        return false;
    }

    int index = m_protocolModels.size();

    beginInsertChild(index);
    beginInsertRows(QModelIndex(), index, index);
    m_protocolModels.insert(index, protocolModel);
    endInsertRows();
    endInsertChild();

    sort();
    return true;
}

bool TProtocolContainer::add(const TProtocol & protocol) {
    bool nameFound;
    getByName(protocol.getName(), &nameFound);

    if(nameFound) {
        return false;
    }

    int index = m_protocolModels.size();

    beginInsertChild(index);
    beginInsertRows(QModelIndex(), index, index);
    m_protocolModels.insert(index, new TProtocolModel(protocol, this));
    endInsertRows();
    endInsertChild();

    sort();
    return true;
}

bool TProtocolContainer::update(int index, const TProtocol & protocol) {
    if(index >= 0 && index <= m_protocolModels.size()) {
        m_protocolModels[index]->setProtocol(protocol);
        emit dataChanged(QAbstractTableModel::index(index, 0), QAbstractTableModel::index(index, this->columnCount()));

        sort();
        return true;
    }

    return false;
}

bool TProtocolContainer::remove(int index) {
    if(index >= 0 && index < m_protocolModels.size()) {
        beginRemoveChild(index);
        beginRemoveRows(QModelIndex(), index, index);
        delete m_protocolModels[index];
        m_protocolModels.removeAt(index);
        endRemoveRows();
        endRemoveChild();

        emit protocolsUpdated();
        return true;
    }

    return false;
}

void TProtocolContainer::sort() {
    int i, j, n = m_protocolModels.size();
    bool swapped;
    for (i = 0; i < n - 1; i++) {
        swapped = false;
        for (j = 0; j < n - i - 1; j++) {
            if (m_protocolModels[j]->name().compare(m_protocolModels[j + 1]->name(), Qt::CaseInsensitive) > 0) {
                m_protocolModels.swapItemsAt(j, j + 1);
                emit dataChanged(QAbstractTableModel::index(j, 0), QAbstractTableModel::index(j, this->columnCount()));
                emit dataChanged(QAbstractTableModel::index(j + 1, 0), QAbstractTableModel::index(j + 1, this->columnCount()));
                swapped = true;
            }
        }

        if (swapped == false)
            break;
    }

    itemDataChanged();
    emit protocolsUpdated();
}

int TProtocolContainer::rowCount(const QModelIndex &parent) const {
    return count();
}

int TProtocolContainer::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant TProtocolContainer::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_protocolModels.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
        case 0:
            return ((TProtocolModel *)m_protocolModels[index.row()])->protocol().getName();
        case 1:
            return ((TProtocolModel *)m_protocolModels[index.row()])->protocol().getDescription();
        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant TProtocolContainer::headerData(int section, Qt::Orientation orientation, int role) const {
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


int TProtocolContainer::childrenCount() const
{
    return count();
}

TProjectItem * TProtocolContainer::child(int row) const
{
    return m_protocolModels[row];
}

QString TProtocolContainer::name() const
{
    return tr("Protocols");
}

TProjectItem::Status TProtocolContainer::status() const
{
    return Status::None;
}


int TProtocolContainer::loadProtocolFromFile() {
    QStringList filters;
    filters << "TraceXpert protocol file (*.txpp)"
            << "Any files (*)";

    QFileDialog openDialog;
    openDialog.setNameFilters(filters);
    openDialog.setAcceptMode(QFileDialog::AcceptOpen);
    openDialog.setFileMode(QFileDialog::ExistingFiles);

    // TODO: open specific directory?
    // openDialog.setDirectory(m_projectDirectory);

    if (!openDialog.exec()) return 0;

    QStringList files = openDialog.selectedFiles();

    for (const QString &fileName : files) {
        QFile protocolFile(fileName);
        protocolFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QByteArray documentArray = protocolFile.readAll();
        protocolFile.close();

        QDomDocument document;
        document.setContent(documentArray);

        QDomElement projectElement = document.documentElement();
        TProtocolModel * protocolModel = new TProtocolModel(this);

        try {
            protocolModel->load(&projectElement);
            add(protocolModel);
        }
        catch (QString message) {
            if (protocolModel)
                delete protocolModel;
            throw message;
        }
    }

    return files.count();
}

int TProtocolContainer::saveProtocolToFile(const TProtocolModel * protocolModel) {
    QDomDocument document;
    document.appendChild(protocolModel->save(document));

    QStringList filters;
    filters << "TraceXpert protocol file (*.txpp)"
            << "Any files (*)";

    QFileDialog saveDialog;
    saveDialog.setNameFilters(filters);
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setFileMode(QFileDialog::AnyFile);

    // TODO: open specific directory?
    // openDialog.setDirectory(m_projectDirectory);

    saveDialog.selectFile(protocolModel->name());
    if (!saveDialog.exec()) return 0;

    QFile projectFile(saveDialog.selectedFiles().constFirst());
    projectFile.open(QIODevice::WriteOnly | QIODevice::Text);
    projectFile.write(document.toByteArray());
    projectFile.close();

    return 1;
}
