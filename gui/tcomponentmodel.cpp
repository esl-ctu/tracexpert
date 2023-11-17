#include "tcomponentmodel.h"

#include "tcomponentcontainer.h"

TComponentModel::TComponentModel(TPlugin * plugin, TComponentContainer * parent)
    : TProjectItem(parent->model(), parent), TPluginUnitModel(parent), m_plugin(plugin)
{
    m_IOdevices = new TIODeviceContainer(this);
    m_scopes = new TScopeContainer(this);

    m_name = plugin->getPluginName();
    m_info = plugin->getPluginInfo();
}

bool TComponentModel::init()
{
    if (isInit()) {
        return false;
    }

    bool ok;
    m_plugin->init(&ok);

    if (ok) {
        m_isInit = true;
        QList<TIODevice *> IODevices = m_plugin->getIODevices();
        for (int i = 0; i < IODevices.length(); i++) {
            appendIODevice(IODevices[i]);
        }

        QList<TScope *> scopes = m_plugin->getScopes();
        for (int i = 0; i < scopes.length(); i++) {
            appendScope(scopes[i]);
        }
    }

    return ok;
}

bool TComponentModel::deInit()
{
    if (!isInit()) {
        return false;
    }
    
    for (int i = 0; i < m_IOdevices->count(); i++) {
        if (!m_IOdevices->at(i)->deInit()) {
            return false;
        }
    }
    
    for (int i = 0; i < m_scopes->count(); i++) {
        if (!m_scopes->at(i)->deInit()) {
            return false;
        }
    }

    bool ok;
    m_plugin->deInit(&ok);

    if (ok) {
        m_isInit = false;
    }

    return ok;
}

TConfigParam TComponentModel::preInitParams() const
{
    return m_plugin->getPreInitParams();
}

TConfigParam TComponentModel::postInitParams() const
{
    return m_plugin->getPostInitParams();
}

TConfigParam TComponentModel::setPreInitParams(const TConfigParam & param)
{
    return m_plugin->setPreInitParams(param);
}

TConfigParam TComponentModel::setPostInitParams(const TConfigParam & param)
{
    return m_plugin->setPostInitParams(param);
}

int TComponentModel::IODeviceCount() const
{
    return m_IOdevices->count();
}

int TComponentModel::scopeCount() const
{
    return m_scopes->count();
}

TIODeviceModel * TComponentModel::IODevice(int index) const
{
    return m_IOdevices->at(index);
}

TScopeModel * TComponentModel::scope(int index) const
{
    return m_scopes->at(index);
}

bool TComponentModel::canAddIODevice() const
{
    return m_plugin->canAddIODevice();
}

bool TComponentModel::canAddScope() const
{
    return m_plugin->canAddScope();
}

bool TComponentModel::addIODevice(QString name, QString info)
{
    if (m_plugin->canAddIODevice()) {
        bool ok;
        TIODevice * IODevice = m_plugin->addIODevice(name, info, &ok);

        if (ok) {
            appendIODevice(IODevice);
            return true;
        }
    }

    return false;
}

bool TComponentModel::addScope(QString name, QString info)
{
    if (m_plugin->canAddScope()) {
        bool ok;
        TScope * scope = m_plugin->addScope(name, info, &ok);

        if (ok) {
            appendScope(scope);
            return true;
        }
    }

    return false;
}

TIODeviceContainer * TComponentModel::IODeviceContainer() const
{
    return m_IOdevices;
}

TScopeContainer * TComponentModel::scopeContainer() const
{
    return m_scopes;
}

int TComponentModel::childrenCount() const
{
    int count = 0;

    if (m_IOdevices) {
        count++;
    }
    if (m_scopes) {
        count++;
    }

    return count;
}

TProjectItem * TComponentModel::child(int row) const
{
    if (m_IOdevices) {
        if (row == 0) {
            return m_IOdevices;
        }
        else {
            row--;
        }
    }

    if (m_scopes) {
        if (row == 0) {
            return m_scopes;
        }
        else {
            row--;
        }
    }

    return nullptr;
}

QVariant TComponentModel::status() const
{
    if (m_isInit) {
        return tr("Initialized");
    }
    else {
        return tr("Uninitialized");
    }
}

void TComponentModel::appendIODevice(TIODevice * IODevice)
{
    TIODeviceModel * IODeviceModel = new TIODeviceModel(IODevice, m_IOdevices);
    connect(IODeviceModel, &TIODeviceModel::initialized, this, &TComponentModel::IODeviceInitialized);
    connect(IODeviceModel, &TIODeviceModel::deinitialized, this, &TComponentModel::IODeviceDeinitialized);
    m_IOdevices->add(IODeviceModel);
}

void TComponentModel::appendScope(TScope * scope)
{
    TScopeModel * scopeModel = new TScopeModel(scope, m_scopes);
    connect(scopeModel, &TScopeModel::initialized, this, &TComponentModel::scopeInitialized);
    connect(scopeModel, &TScopeModel::deinitialized, this, &TComponentModel::scopeDeinitialized);
    m_scopes->add(scopeModel);
}
