#include "tscopecontainer.h"

#include "tcomponentmodel.h"

TScopeContainer::TScopeContainer(TComponentModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{

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

    beginInsertRows(QModelIndex(), m_scopes.length(), m_scopes.length());
    m_scopes.append(unit);
    endInsertRows();
}

QString TScopeContainer::name() const
{
    return tr("Oscilloscopes");
}
