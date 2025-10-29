#include "tcomponentcontainer.h"

#include "../../project/tprojectmodel.h"

TComponentContainer::TComponentContainer(TProjectModel * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitContainer(parent)
{
    m_typeName = "components";
}

int TComponentContainer::count() const
{
    return m_components.length();
}

TComponentModel * TComponentContainer::at(int index) const
{
    if (index >= 0 && index < count()) {
        return m_components[index];
    }
    else {
        return nullptr;
    }
}

TComponentModel * TComponentContainer::getByName(const QString &name) const {
    for(TComponentModel * componentModel : m_components) {
        if(componentModel->name() == name) {
            return componentModel;
        }
    }

    return nullptr;
}

void TComponentContainer::add(TComponentModel * unit)
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
