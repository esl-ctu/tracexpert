#include "tpluginunitmodel.h"

TPluginUnitModel::TPluginUnitModel(QObject * parent)
    : QObject(parent)
{
    m_isInit = false;
}

bool TPluginUnitModel::isInit() const
{
    return m_isInit;
}
