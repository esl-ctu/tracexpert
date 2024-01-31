#include "tprojectmodel.h"

#include <QDir>
#include <QCoreApplication>
#include <QPluginLoader>

TProjectModel::TProjectModel(QObject * parent)
    : QAbstractItemModel(parent), TProjectItem(this, nullptr)
{
    loadComponents();
    loadProtocols();
}

TProjectModel::~TProjectModel()
{

}

TComponentContainer *TProjectModel::componentContainer()
{
    return m_componentContainer;
}

TProtocolContainer *TProjectModel::protocolContainer()
{
    return m_protocolContainer;
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

    parentItem = static_cast<TProjectItem *>(parent.internalPointer());

    const TProjectItem * childItem = parentItem ? parentItem->child(row) : this;

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

    if (m_protocolContainer) {
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

    if (m_protocolContainer) {
        if (row == 0) {
            return m_protocolContainer;
        }
        else {
            row--;
        }
    }

    return nullptr;
}

QString TProjectModel::name() const
{
    return tr("Project");
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

void TProjectModel::loadProtocols()
{
    m_protocolContainer = new TProtocolContainer(this);

    QList<TMessage> messages;

    QList<TMessage> messages1;
    TMessage messageC("Dummy protocol message (command)", "Description of dummy protocol message.", false);
    TMessage messageR("Dummy protocol message (response)", "Description of dummy protocol message.", true);
    TMessagePart messagePart0("Foo", "Lorem ipsum.", TMessagePart::TType::TBool);
    messageC.addMessagePart(messagePart0);
    messageR.addMessagePart(messagePart0);
    TMessagePart messagePart1("Bar", "Lorem ipsum.", TMessagePart::TType::TString);
    messagePart1.setLength(4);
    messageC.addMessagePart(messagePart1);
    messageR.addMessagePart(messagePart1);
    TMessagePart messagePart2("Goop", "Lorem ipsum.", TMessagePart::TType::TInt);
    messageC.addMessagePart(messagePart2);
    messageR.addMessagePart(messagePart2);
    messages1.append(messageC);
    messages1.append(messageR);

    QList<TMessage> messages2;

    m_protocolContainer->insertItem(0, new TProtocolModel(TProtocol("Dummy protocol 3", "Description of dummy protocol.", messages), m_protocolContainer));
    m_protocolContainer->insertItem(0, new TProtocolModel(TProtocol("Dummy protocol 2", "Description of dummy protocol.", messages2), m_protocolContainer));
    m_protocolContainer->insertItem(0, new TProtocolModel(TProtocol("Dummy protocol 1", "Description of dummy protocol.", messages1), m_protocolContainer));
}
