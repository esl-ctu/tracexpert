#include "tpluginunitmodel.h"

TPluginUnitModel::TPluginUnitModel(TCommon * unit, bool manual, QObject * parent)
    : QObject(parent), m_unit(unit), m_isManual(manual)
{
    m_isInit = false;
    m_wasInit = false;
    m_initWhenAvailable = false;

    if (m_unit) {
        m_name = m_unit->getName();
        m_info = m_unit->getInfo();
    }
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

bool TPluginUnitModel::initWhenAvailable() const
{
    return m_initWhenAvailable;
}

bool TPluginUnitModel::init()
{
    if (!m_unit)
        return false;

    bool ok;
    m_unit->init(&ok);

    if (ok) {
        m_wasInit = false;
    }

    if (!m_postInitParam.isEmpty()) {
        m_unit->setPostInitParams(m_postInitParam);
    }

    return ok;
}

bool TPluginUnitModel::deInit()
{
    if (!m_unit)
        return false;

    bool ok;
    m_unit->deInit(&ok);

    if (ok) {
        m_preInitParam = preInitParams();
        m_postInitParam = postInitParams();
        m_wasInit = true;
    }

    return ok;
}

bool TPluginUnitModel::isAvailable() const
{
    return m_unit;
}

bool TPluginUnitModel::isManual() const
{
    return m_isManual;
}

TConfigParam TPluginUnitModel::preInitParams() const
{
    return m_wasInit ? m_preInitParam : m_unit->getPreInitParams();
}

TConfigParam TPluginUnitModel::postInitParams() const
{
    return m_wasInit ? m_postInitParam : m_unit->getPostInitParams();
}

TConfigParam TPluginUnitModel::setPreInitParams(const TConfigParam &param)
{
    return m_unit->setPreInitParams(param);
}

TConfigParam TPluginUnitModel::setPostInitParams(const TConfigParam &param)
{
    return m_unit->setPostInitParams(param);
}

bool TPluginUnitModel::toBeSaved() const
{
    return m_wasInit || m_isInit || m_isManual;
}

QDomElement TPluginUnitModel::save(QDomDocument & document) const
{
    QDomElement element = TProjectItem::save(document);

    element.setAttribute("name", name());
    element.setAttribute("info", info());
    element.setAttribute("init", isInit());
    element.setAttribute("manual", isManual());
    element.setAttribute("preinitparams", saveParam(preInitParams()));
    element.setAttribute("postinitparams", saveParam(postInitParams()));

    return element;
}

void TPluginUnitModel::load(QDomElement * element)
{
    if (!element)
        return;

    if (element->tagName() != typeName())
        throw tr("Unexpected tag");

    if (m_isInit)
        throw tr("Cannot load initialized unit");

    m_name = element->attribute("name");

    if (m_name.isEmpty())
        throw tr("Missing name");

    m_info = element->attribute("info");

    QString isManualString = element->attribute("manual");

    if (isManualString.isEmpty())
        throw tr("Missing is manual attribute");

    bool ok;
    m_isManual = isManualString.toInt(&ok);

    if (!ok)
        throw tr("Unexpected value of component is manual attribute");

    m_preInitParam = loadParam(element->attribute("preinitparams").toUtf8());
    m_postInitParam = loadParam(element->attribute("postinitparams").toUtf8());

    m_wasInit = true;

    QString isInitString = element->attribute("init");

    if (isInitString.isEmpty())
        throw tr("Missing is init attribute");

    m_initWhenAvailable = isInitString.toInt(&ok);

    if (!ok)
        throw tr("Unexpected value of init attribute");
}

void TPluginUnitModel::bind(TCommon * unit)
{
    m_unit = unit;
}

void TPluginUnitModel::release()
{
    m_unit = nullptr;
}

TProjectItem::Status TPluginUnitModel::status() const
{
    if (!isAvailable())
        return Unavailable;

    if (m_isInit) {
        return Initialized;
    }
    else {
        return Uninitialized;
    }
}

QByteArray TPluginUnitModel::saveParam(const TConfigParam & param) const
{
    QByteArray array;
    QDataStream stream(&array, QIODeviceBase::WriteOnly);
    stream << param;
    return array.toBase64();
}

TConfigParam TPluginUnitModel::loadParam(const QByteArray & array) const
{
    QDataStream stream(QByteArray::fromBase64(array));
    TConfigParam param;
    stream >> param;
    return param;
}
