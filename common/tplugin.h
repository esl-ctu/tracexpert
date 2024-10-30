#ifndef TPLUGIN_H
#define TPLUGIN_H

#include <QtPlugin>
#include <QString>
#include <QList>
#include "tconfigparam.h"
#include "tiodevice.h"
#include "tscope.h"
#include "tanaldevice.h"
#include "tcommon.h"

class TPlugin : public TCommon {

public:

    virtual ~TPlugin() {} // Should check if deinit() was done

    /// Plugin name
    virtual QString getName() const = 0;
    /// Plugin info
    virtual QString getInfo() const = 0;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const = 0;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) = 0;

    /// Initialize the plugin
    virtual void init(bool *ok = nullptr) = 0;
    /// Deinitialize the plugin
    virtual void deInit(bool *ok = nullptr) = 0;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const = 0;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) = 0;

    /// Add an IO Device manually
    virtual TIODevice * addIODevice(QString name, QString info, bool *ok = nullptr) = 0;
    /// Add a Scope manually
    virtual TScope * addScope(QString name, QString info, bool *ok = nullptr) = 0;
    /// Add an Analytical device manually
    virtual TAnalDevice * addAnalDevice(QString name, QString info, bool *ok = nullptr) = 0;

    /// Returns true, when it is possible to add an IO Device manually
    virtual bool canAddIODevice() = 0;
    /// Returns true, when it is possible to add a Scope manually
    virtual bool canAddScope() = 0;
    /// Returns true, when it is possible to add an Analytical device manually
    virtual bool canAddAnalDevice() = 0;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() = 0;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() = 0;
    /// Get available Analytical devices, available only after init()
    virtual QList<TAnalDevice *> getAnalDevices() = 0;

};


#define TPlugin_iid "org.cvut.fit.TraceXpert.PluginInterface/1.0"

Q_DECLARE_INTERFACE(TPlugin, TPlugin_iid)

#endif // TPLUGIN_H
