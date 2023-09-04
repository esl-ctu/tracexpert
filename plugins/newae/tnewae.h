
#ifndef NEWAE_H
#define NEWAE_H
#pragma once

#include "tnewae_global.h"
#include "tplugin.h"
#include "tnewaedevice.h"
#include "tnewaescope.h"

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

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//! THIS CLASS IS NOT THREAD SAFE !
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

const std::size_t ADDR_SIZE = 16;
const std::size_t SM_SIZE_ADDR = 0;
const std::size_t SM_DATA_ADDR = SM_SIZE_ADDR + ADDR_SIZE;
const int PROCESS_WAIT_MSCECS = 10000;
const char fieldSeparator = ',';
const char lineSeparator = '\n';
const uint8_t NO_CW_ID = 255;

//All interprocess communication is ASCII
//All shared memory binary (size is size_t, data are uint8_t)

//Special functions implemented (to implement) in python - HALT, SETUP, DETECTDEVICES, SMTEST. FUNC-<pythonFunctionName>
//Special codes to be received from python - STARTED, DONE, ERROR

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

    virtual TIODevice * addIODevice(QString name, QString info, bool *ok = nullptr) override;
    virtual TScope * addScope(QString name, QString info, bool *ok = nullptr) override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;

    /// Returns true, when it is possible to add an IO Device manually
    virtual bool canAddIODevice() override;
    /// Returns true, when it is possible to add a Scope manually
    virtual bool canAddScope() override;

    TnewaeScope * getCWScopeObjectById(uint8_t id);

    //In this block, cwId is only used for identification if the correct CW is being accessed
    bool writeToPython(uint8_t cwId, const QString &data, bool responseExpected = true, bool wait = true);
    bool readFromPython(uint8_t cwId, QString &data, bool wait = true);
    bool waitForPythonDone(uint8_t cwId, bool discardOutput, int timeout = 30000);

    //In this block, CW is super important
    void packageDataForPython(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out);
    bool runPythonFunctionAndGetStringOutput(int8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, size_t &dataLen, QString &out);
    bool getPythonParameter(int8_t cwId, QString paramName, QString &out);
    bool getPythonSubparameter(int8_t cwId, QString paramName, QString subParamName, QString &out);
    bool setPythonParameter(int8_t cwId, QString paramName, QString value, QString &out); //Out is the new value of the parameter, can be discarded
    bool setPythonSubparameter(int8_t cwId, QString paramName, QString subParamName, QString value, QString &out); //Out is the new value of the subparameter, can be discarded

public slots:
    static void handlePythonError(QProcess::ProcessError error);
    void checkForPythonState();

protected:
    void _createPreInitParams();
    bool _validatePreInitParamsStructure(TConfigParam & params);

    const QString PLUGIN_ID = "TraceXpert.NewAE";

    //Methods for setup:
    bool setUpSHM();
    bool setUpPythonProcess();
    bool testSHM();
    bool autodetectDevices(QList<std::pair<QString, QString>> & devices);

    bool getDataFromShm(size_t &size, QString &data);

    //In this block, CW is super important
    void packagePythonFunction(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out);
    void packagePythonParam(uint8_t cwId, QString paramName, QString value, QString &out);
    void packagePythonSubparam(uint8_t cwId, QString paramName, QString subParamName, QString value, QString &out);

    uint8_t numDevices; //This counts the number of **seen** devices, not the number of connected devices. Use m_scopes.lenght() for that
    bool pythonReady;
    bool pythonError;
    bool deviceWaitingForRead;
    uint8_t waitingForReadDeviceId;
    uint8_t lastCWActive;
    QString pythonPath;

    QList<TIODevice *> m_ports;
    QList<TScope *> m_scopes;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;

    QSharedMemory shm;
    QProcess *pythonProcess;

    QString shmKey = PLUGIN_ID + "shm2";
    const size_t shmSize = 1024*1024*1024;
};


#endif // NEWAE_H
