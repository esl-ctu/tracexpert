#include "tprotocolcontainer.h"
#include "tprotocolmodel.h"
#include "tprojectmodel.h"

TProtocolContainer::TProtocolContainer(TProjectModel * parent) : TAbstractTableModel<TProtocolModel *>(parent), TProjectItem(parent->model(), parent) { }

TProtocol TProtocolContainer::getProtocolByName(const QString &name, bool *ok) {
    for(TProtocolModel * protocolModel : m_items) {
        if(protocolModel->protocol().getName() == name) {
            if(ok != nullptr) *ok = true;
            return protocolModel->protocol();
        }
    }

    if(ok != nullptr) *ok = false;
    return TProtocol();
}

int TProtocolContainer::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant TProtocolContainer::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (index.row() >= m_items.size())
        return QVariant();

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
        case 0:
            return ((TProtocolModel *)m_items[index.row()])->protocol().getName();
        case 1:
            return ((TProtocolModel *)m_items[index.row()])->protocol().getDescription();
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
    return m_items.size();
}

TProjectItem * TProtocolContainer::child(int row) const
{
    return m_items[row];
}

QString TProtocolContainer::name() const
{
    return tr("Protocols");
}

QVariant TProtocolContainer::status() const
{
    return QString();
}
