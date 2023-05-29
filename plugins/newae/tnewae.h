
//#ifndef NEWAE_H
//#define NEWAE_H
#pragma once

#include "tnewae_global.h"
#include "tplugin.h"
#include "tnewaedevice.h"

#include <QCoreApplication>
#include <QSharedMemory>
#include <QWaitCondition>
#include <QProcess>
#include <QList>
#include <QTextStream>
#include <QDir>
#include <QtDebug>
#include <QFile>
#include <QRandomGenerator>

const std::size_t ADDR_SIZE = 16;
const std::size_t SM_SIZE_ADDR = 0;
const std::size_t SM_DATA_ADDR = SM_SIZE_ADDR + ADDR_SIZE;
const int PROCESS_WAIT_MSCECS = 10000;
const char fieldSeparator = ',';
const char lineSeparator = '\n';

//All interprocess communication is ASCII
//All shared memory binary (size is size_t, data are uint8_t)

//Special functions implemented (to implement) in python - HALT, INIT_DEVICE
//Special codes to be received from python - STARTED, (DONE)

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

    //cwId is only used for identification if the correct CW is being accessed
    bool writeToPython(uint8_t cwId, const QString &data, bool responseExpected = true, bool wait = true);
    bool readFromPython(uint8_t cwId, QString &data, bool wait = true);
    bool checkForPythonReady(int wait = 30000);
    bool checkForPythonError(); //All output is discarded on error
    bool waitForPythonDone(uint8_t cwId, int timeout = 30000);

    void packageDataForPython(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out);

protected:
    const QString PLUGIN_ID = "TraceXpert.NewAE";

    bool getDataFromShm(size_t &size, QString &data);

    uint8_t numDevices;
    bool pythonReady;
    bool deviceWaitingForRead;
    uint8_t waitingForReadDeviceId;

    QList<TIODevice *> m_ports;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;

    QSharedMemory shm;
    QProcess *pythonProcess;

    QString shmKey = PLUGIN_ID + "shm";
    const size_t shmSize = 1024*1024*1024;
};


//#endif // NEWAE_H
