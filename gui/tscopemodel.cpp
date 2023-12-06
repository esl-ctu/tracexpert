#include "tscopemodel.h"

#include "tscopecontainer.h"

TScopeModel::TScopeModel(TScope * scope, TScopeContainer * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitModel(parent), m_scope(scope)
{
    m_name = m_scope->getName();
    m_info = m_scope->getInfo();
}

bool TScopeModel::init()
{
    if (isInit()) {
        return false;
    }

    bool ok;
    m_scope->init(&ok);

    if (ok) {
        m_isInit = true;
    }

    return ok;
}

bool TScopeModel::deInit()
{
    if (!isInit()) {
        return false;
    }

    bool ok;
    m_scope->deInit(&ok);

    if (ok) {
        m_isInit = false;
    }

    return ok;
}

TConfigParam TScopeModel::preInitParams() const
{
    return m_scope->getPreInitParams();
}

TConfigParam TScopeModel::postInitParams() const
{
    return m_scope->getPostInitParams();
}

TConfigParam TScopeModel::setPreInitParams(const TConfigParam & param)
{
    return m_scope->setPreInitParams(param);
}

TConfigParam TScopeModel::setPostInitParams(const TConfigParam & param)
{
    return m_scope->setPostInitParams(param);
}

int TScopeModel::childrenCount() const
{
    return 0;
}

TProjectItem *TScopeModel::child(int row) const
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
