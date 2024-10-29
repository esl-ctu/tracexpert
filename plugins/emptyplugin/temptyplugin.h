#ifndef TEMPTYPLUGIN_H
#define TEMPTYPLUGIN_H

#include "TEmptyPlugin_global.h"
#include "tplugin.h"


class TEMPTYPLUGIN_EXPORT TEmptyPlugin : public QObject, TPlugin
{

    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "temptyplugin.json")
    Q_INTERFACES(TPlugin)

public:
    TEmptyPlugin();
    virtual ~TEmptyPlugin() override;

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

    /// Add an IO Device manually
    virtual TIODevice * addIODevice(QString name, QString info, bool *ok = nullptr) override;
    /// Add a Scope manually
    virtual TScope * addScope(QString name, QString info, bool *ok = nullptr) override;
    /// Add a Analytical device manually
    virtual TAnalDevice * addAnalDevice(QString name, QString info, bool *ok = nullptr) override;

    /// Returns true, when it is possible to add an IO Device manually
    virtual bool canAddIODevice() override;
    /// Returns true, when it is possible to add a Scope manually
    virtual bool canAddScope() override;
    /// Returns true, when it is possible to add an Analytical device manually
    virtual bool canAddAnalDevice() override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;
    /// Get available Analytical devices, available only after init()
    virtual QList<TAnalDevice *> getAnalDevices() override;

protected:



};

#endif // TEMPTYPLUGIN_H
