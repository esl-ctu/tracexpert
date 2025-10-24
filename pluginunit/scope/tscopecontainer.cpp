#include "tscopecontainer.h"

#include "../component/tcomponentmodel.h"

TScopeContainer::TScopeContainer(TComponentModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{
    m_typeName = "scopes";
}

int TScopeContainer::count() const
{
    return m_scopes.length();
}

TScopeModel * TScopeContainer::at(int index) const
{
    if (index >= 0 && index < count()) {
        return m_scopes[index];
    }
    else {
        return nullptr;
    }
}

void TScopeContainer::add(TScopeModel * unit)
{
    unit->setParent(this);

    beginInsertChild(m_scopes.length());
    beginInsertRows(QModelIndex(), m_scopes.length(), m_scopes.length());
    m_scopes.append(unit);
    endInsertRows();
    endInsertChild();
}

bool TScopeContainer::remove(TScopeModel * unit)
{
    int index = m_scopes.indexOf(unit);

    if (index < 0) {
        return false;
    }

    beginRemoveChild(index);
    beginRemoveRows(QModelIndex(), index, index);
    m_scopes.remove(index);
    endRemoveRows();
    endRemoveChild();

    return true;
}

TScopeModel * TScopeContainer::hasName(QString name) const
{
    for (int i = 0; i < m_scopes.length(); i++) {
        if (m_scopes[i]->name() == name) {
            return m_scopes[i];
        }
    }

    return nullptr;
}

QString TScopeContainer::name() const
{
    return tr("Oscilloscopes");
}
