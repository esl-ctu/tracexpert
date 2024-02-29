#ifndef TCOMPONENTMODEL_H
#define TCOMPONENTMODEL_H

#include <QObject>

#include "tpluginunitmodel.h"
#include "tplugin.h"
#include "tiodevicemodel.h"
#include "tscopemodel.h"
#include "tiodevicecontainer.h"
#include "tscopecontainer.h"

class TComponentContainer;

class TComponentModel : public TPluginUnitModel
{
    Q_OBJECT

public:
    explicit TComponentModel(TPlugin * plugin, TComponentContainer * parent);

    bool init() override;
    bool deInit() override;

    int IODeviceCount() const;
    int scopeCount() const;

    TIODeviceModel * IODevice(int index) const;
    TScopeModel * scope(int index) const;

    bool canAddIODevice() const;
    bool canAddScope() const;

    bool addIODevice(QString name, QString info);
    bool addScope(QString name, QString info);

    TIODeviceContainer * IODeviceContainer() const;
    TScopeContainer * scopeContainer() const;

    int childrenCount() const override;
    TProjectItem * child(int row) const override;

    virtual void load(QDomElement * element) override;

signals:
    void IODeviceInitialized(TIODeviceModel * IODevice);
    void IODeviceDeinitialized(TIODeviceModel * IODevice);

    void scopeInitialized(TScopeModel * scope);
    void scopeDeinitialized(TScopeModel * scope);

private:
    void appendIODevice(TIODevice * IODevice, QDomElement * element = nullptr);
    void appendScope(TScope * scope, QDomElement * element = nullptr);

    void loadIODevices(QDomElement * element);
    void loadIODevice(QDomElement * element);
    void loadScopes(QDomElement * element);
    void loadScope(QDomElement * element);

    TPlugin * m_plugin;

    TIODeviceContainer * m_IOdevices;
    TScopeContainer * m_scopes;
};

#endif // TCOMPONENTMODEL_H
