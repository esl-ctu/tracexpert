#include "tscopemodel.h"

#include "tscopecontainer.h"

TScopeModel::TScopeModel(TScope * scope, TScopeContainer * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitModel(scope, parent), m_scope(scope)
{
}

bool TScopeModel::init()
{
    if (isInit() || !TPluginUnitModel::init()) {
        return false;
    }

    m_isInit = true;

    return true;
}

bool TScopeModel::deInit()
{
    if (!isInit() || !TPluginUnitModel::deInit()) {
        return false;
    }

    m_isInit = false;

    return true;
}

int TScopeModel::childrenCount() const
{
    return 0;
}

TProjectItem * TScopeModel::child(int row) const
{
    return nullptr;
}

QVariant TScopeModel::status() const
{
    if (m_isInit) {
        return tr("Initialized");
    }
    else {
        return tr("Uninitialized");
    }
}
