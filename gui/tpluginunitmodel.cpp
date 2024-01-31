#include "tpluginunitmodel.h"

TPluginUnitModel::TPluginUnitModel(TCommon * unit, QObject * parent)
    : QObject(parent), m_unit(unit)
{
    m_isInit = false;

    m_name = m_unit->getName();
    m_info = m_unit->getInfo();
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

bool TPluginUnitModel::init()
{
    bool ok;
    m_unit->init(&ok);
    return ok;
}

bool TPluginUnitModel::deInit()
{
    bool ok;
    m_unit->deInit(&ok);
    return ok;
}

TConfigParam TPluginUnitModel::preInitParams() const
{
    return m_unit->getPreInitParams();
}

TConfigParam TPluginUnitModel::postInitParams() const
{
    return m_unit->getPostInitParams();
}

TConfigParam TPluginUnitModel::setPreInitParams(const TConfigParam &param)
{
    return m_unit->setPreInitParams(param);
}

TConfigParam TPluginUnitModel::setPostInitParams(const TConfigParam &param)
{
    return m_unit->setPostInitParams(param);
}
