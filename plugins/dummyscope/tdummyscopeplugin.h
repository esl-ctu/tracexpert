#ifndef TDUMMYSCOPEPLUGIN_H
#define TDUMMYSCOPEPLUGIN_H

#include "TDummyScopePlugin_global.h"
#include "tplugin.h"


class TDUMMYSCOPEPLUGIN_EXPORT TDummyScopePlugin : public QObject, TPlugin
{

    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "tdummyscopeplugin.json")
    Q_INTERFACES(TPlugin)

public:
    TDummyScopePlugin();
    virtual ~TDummyScopePlugin() override;

    /// Plugin name
    virtual QString getName() const override;
    /// Plugin info
    virtual QString getInfo() const override;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const override;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    /// Initialize the plugin
    virtual void init(bool *ok = nullptr) override;
    /// Deinitialize the plugin
    virtual void deInit(bool *ok = nullptr) override;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const override;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual TIODevice * addIODevice(QString name, QString info, bool *ok = nullptr) override;
    virtual TScope * addScope(QString name, QString info, bool *ok = nullptr) override;
    virtual TAnalDevice * addAnalDevice(QString name, QString info, bool *ok = nullptr) override;

    virtual bool canAddIODevice() override;
    virtual bool canAddScope() override;
    virtual bool canAddAnalDevice() override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;
    /// Get available Analytical devices, available only after init()
    virtual QList<TAnalDevice *> getAnalDevices() override;

};

#endif // TDUMMYSCOPEPLUGIN_H
