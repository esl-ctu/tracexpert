#include "tpluginunitmodel.h"

TPluginUnitModel::TPluginUnitModel(QObject * parent)
    : QObject(parent)
{
    m_isInit = false;
}

QString TPluginUnitModel::name() const
{
    return m_name;
}

QString TPluginUnitModel::info() const
{
    return m_info;
}

bool TPluginUnitModel::isInit() const
{
    return m_isInit;
}
