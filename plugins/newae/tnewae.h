
#ifndef NEWAE_H
#define NEWAE_H

#include "tnewae_global.h"
#include "tplugin.h"
#include "tnewaedevice.h"

#include <QSharedMemory>
#include <QWaitCondition>
#include <QProcess>
#include <QList>
#include <QTextStream>

const std::size_t SM_SIZE_ADDR = 0;
const std::size_t SM_DATA_ADDR = SM_SIZE_ADDR + sizeof(SM_SIZE_ADDR);
const char fieldSeparator = ',';
const char lineSeparator = '\r';

//All interprocess communication is ASCII
//All shared memory binary (size is size_t, data are uint8_t)

class TNEWAE_EXPORT TNewae : public QObject, TPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "tnewae.json")
    Q_INTERFACES(TPlugin)
public:
    TNewae();
    virtual ~TNewae() override;

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
    const QString PLUGIN_ID = "TraceXpert.NewAE";

    void packageDataForPython(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out);
    bool getDataFromShm(size_t &size, QList<uint8_t> &data);

    QList<TIODevice *> m_ports;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;

    QSharedMemory shm;
    QProcess *pythonProcess;

    QString shmKey = PLUGIN_ID + "shm";
    const size_t shmSize = 512;
};


#endif // NEWAE_H
