#include "tprojectmodel.h"

#include <QDir>
#include <QCoreApplication>
#include <QPluginLoader>

TProjectModel::TProjectModel(QObject * parent)
    : QAbstractItemModel(parent), TProjectItem(this, nullptr)
{
    loadComponents();
}

TProjectModel::~TProjectModel()
{

}

TComponentContainer *TProjectModel::componentContainer()
{
    return m_componentContainer;
}

QVariant TProjectModel::data(const QModelIndex & index, int role) const
{
    if (role == Qt::ItemDataRole::DisplayRole) {
        if (index.column() == 0) {
            return static_cast<TProjectItem *>(index.internalPointer())->name();
        }

        else if (index.column() == 1) {
            return static_cast<TProjectItem *>(index.internalPointer())->status();
        }
    }

    return QVariant();
}

QVariant TProjectModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::ItemDataRole::DisplayRole) {
        if (section == 0) {
            return tr("Name");
        }
        else if (section == 1) {
            return tr("Status");
        }
    }

    return QVariant();
}

QModelIndex TProjectModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    const TProjectItem * parentItem;

    if (!parent.isValid()) {
        parentItem = this;
    }
    else {
        parentItem = static_cast<TProjectItem *>(parent.internalPointer());
    }

    TProjectItem * childItem = parentItem->child(row);

    if (childItem) {
        return createIndex(row, column, childItem);
    }

    return QModelIndex();
}

QModelIndex TProjectModel::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    TProjectItem * childItem = static_cast<TProjectItem *>(index.internalPointer());
    TProjectItem * parentItem = childItem->parent();

    if (!parentItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int TProjectModel::rowCount(const QModelIndex & parent) const
{
    if (!parent.isValid()) {
        return 1;
    }
    else {
        return static_cast<TProjectItem *>(parent.internalPointer())->childrenCount();
    }
}

int TProjectModel::columnCount(const QModelIndex & parent) const
{
    return 2;
}

void TProjectModel::emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit dataChanged(topLeft, bottomRight);
}

int TProjectModel::childrenCount() const
{
    int count = 0;

    if (m_componentContainer) {
        count++;
    }

    return count;
}

TProjectItem * TProjectModel::child(int row) const
{
    if (m_componentContainer) {
        if (row == 0) {
            return m_componentContainer;
        }
        else {
            row--;
        }
    }

    return nullptr;
}

QString TProjectModel::name() const
{
    return QString();
}

QVariant TProjectModel::status() const
{
    return QString();
}

void TProjectModel::loadComponents()
{
    QDir pluginsDir(QCoreApplication::applicationDirPath()+"/plugins");

    QStringList pluginFiles = pluginsDir.entryList(QStringList("T*.dll"), QDir::Files, QDir::Name);

    m_componentContainer = new TComponentContainer(this);

    for (int i = 0; i < pluginFiles.size(); i++) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(pluginFiles.at(i)));
        QObject * pluginInstance = pluginLoader.instance();
        if (pluginInstance) {
            TPlugin * plugin = qobject_cast<TPlugin *>(pluginInstance);
            if (plugin) {
                TComponentModel * component = new TComponentModel(plugin, m_componentContainer);
                connect(component, &TComponentModel::IODeviceInitialized, this, &TProjectModel::IODeviceInitialized);
                connect(component, &TComponentModel::scopeInitialized, this, &TProjectModel::scopeInitialized);
                m_componentContainer->add(component);
            }
        }
    }
}
