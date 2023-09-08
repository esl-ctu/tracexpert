#include "tcomponentcontainer.h"

#include "tprojectmodel.h"

TComponentContainer::TComponentContainer(TProjectModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{

}

int TComponentContainer::unitCount() const
{
    return m_components.length();
}

TComponentModel * TComponentContainer::unit(int index) const
{
    if (index >= 0 && index < unitCount()) {
        return m_components[index];
    }
    else {
        return nullptr;
    }
}

void TComponentContainer::addComponent(TComponentModel * unit)
{
    unit->setParent(this);

    beginInsertChild(m_components.length());
    beginInsertRows(QModelIndex(), m_components.length(), m_components.length());
    m_components.append(unit);
    endInsertRows();
    endInsertChild();
}

QString TComponentContainer::name() const
{
    return tr("Components");
}
