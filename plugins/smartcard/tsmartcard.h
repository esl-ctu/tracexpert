#ifndef SMARTCARD_H
#define SMARTCARD_H

#include "TSmartCard_global.h"
#include "tplugin.h"
#include "tsmartcarddevice.h"
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>


class TSMARTCARD_EXPORT TSmartCard : public QObject, TPlugin
{

    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "tsmartcard.json")
    Q_INTERFACES(TPlugin)

public:
    TSmartCard();
    virtual ~TSmartCard() override;

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

    virtual bool canAddIODevice() override;
    virtual bool canAddScope() override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;

//public slots:
//    void pingCards();

protected:

    QList<TIODevice *> m_readers;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;

};

#endif // SMARTCARD_H
