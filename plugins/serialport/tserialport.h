#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "TSerialPort_global.h"
#include "tplugin.h"
#include "tserialportdevice.h"


class TSERIALPORT_EXPORT TSerialPort : public QObject, TPlugin
{

    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "tserialport.json")
    Q_INTERFACES(TPlugin)

public:
    TSerialPort();
    virtual ~TSerialPort() override;

    /// Plugin name
    virtual QString getPluginName() const override;
    /// Plugin info
    virtual QString getPluginInfo() const override;

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

    virtual void addIODevice(QString name, QString info, bool *ok = nullptr) override;
    virtual void addScope(QString name, QString info, bool *ok = nullptr) override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;

protected:

    QList<TIODevice *> m_ports;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;

};

#endif // SERIALPORT_H
