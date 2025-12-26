// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Tomáš Přeučil (initial author)

#ifndef NEWAE_H
#define NEWAE_H
#pragma once

#include "tnewae_global.h"

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
#include <QMutex>
#include <QMap>

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//! DO NOT SPAWN NEW THREADS WITHIN THIS CLASS !
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class TnewaeScope;
class TnewaeDevice;

const std::size_t ADDR_SIZE = 16;
const std::size_t SM_SIZE_ADDR = 0;
const std::size_t SM_DATA_ADDR = SM_SIZE_ADDR + ADDR_SIZE;
const std::size_t SM_NUM_TRACES_ADDR = 0;
const std::size_t SM_TRACE_SIZE_ADDR = SM_NUM_TRACES_ADDR + ADDR_SIZE;
const std::size_t SM_TRACES_ADDR = SM_TRACE_SIZE_ADDR + ADDR_SIZE;
const int PROCESS_WAIT_MSCECS = 30000;
const char fieldSeparator = ',';
const char lineSeparator = '\n';
const uint8_t NO_CW_ID = 255;

//All interprocess communication is ASCII
//All shared memory binary (size is size_t, data are uint8_t)

//Special functions implemented in python - HALT, SETUP, DETECT_DEVICES, SMTEST, SMSET, DEINI. FUNC-<pythonFunctionName>, FUNO-<objname,funcname>, SPAR-<>, PARA-<>
//Special codes to be received from python - STARTED, DONE, ERROR

class TNEWAE_EXPORT TNewae : public QObject, TPlugin {
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "tnewae.json")
    Q_INTERFACES(TPlugin)
public:
    TNewae();
    virtual ~TNewae() override;

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
    TIODevice * addIODeviceAutomatically(QString name, QString info, targetType type, bool *ok = nullptr);//Never call this manually!
    virtual TScope * addScope(QString name, QString info, bool *ok = nullptr) override;
    TScope * addScopeAutomatically(QString name, QString info, bool *ok = nullptr);//Never call this manually!
    /// Add a Analytical device manually
    virtual TAnalDevice * addAnalDevice(QString name, QString info, bool *ok = nullptr) override;
    uint8_t addDummyScope();


    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;
    /// Get available Analytical devices, available only after init()
    virtual QList<TAnalDevice *> getAnalDevices() override;


    /// Returns true, when it is possible to add an IO Device manually
    virtual bool canAddIODevice() override;
    /// Returns true, when it is possible to add a Scope manually
    virtual bool canAddScope() override;
    /// Returns true, when it is possible to add an Analytical device manually
    virtual bool canAddAnalDevice() override;

    TnewaeScope * getCWScopeObjectById(uint8_t id);

    bool setUpAndTestSHM(uint8_t cwId);

    //In this block, cwId is only used for identification if the correct CW is being accessed
    bool writeToPython(uint8_t cwId, const QString &data, bool asTarget = false, bool responseExpected = true, bool wait = true);
    bool writeBinaryToPython(uint8_t cwId, const char * data, size_t len, bool asTarget = false, bool responseExpected = true, bool wait = true);
    //bool readFromPython(uint8_t cwId, QString &data, bool wait = true);
    bool waitForPythonDone(uint8_t cwId, int timeout = 30000);
    bool waitForPythonTargetDone(uint8_t cwId, int timeout = 30000);

    //In this block, CW is super important
    void packageDataForPython(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out);
    void packageErrorProcessed(uint8_t cwId, QString &out, bool asTarget = false);
    bool sendPythonErrorProcessed(int8_t cwId, bool asTarget = false);
    bool runPythonFunctionAndGetStringOutput(int8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, size_t &dataLen, QString &out, bool asTarget = false);
    bool runPythonFunctionWithBinaryDataAsOneArgumentAndGetStringOutput(int8_t cwId, QString functionName, char * data, size_t lenIn, size_t &dataLen, QString &out, bool asTarget = false);
    bool runPythonFunctionOnAnObjectAndGetStringOutput(int8_t cwId, QString ObjectName, QString functionName, uint8_t numParams, QList<QString> params, size_t &dataLen, QString &out, bool asTarget = false);
    bool getPythonParameter(int8_t cwId, QString paramName, QString &out, bool asTarget = false);
    bool getPythonSubparameter(int8_t cwId, QString paramName, QString subParamName, QString &out, bool asTarget = false);
    bool setPythonParameter(int8_t cwId, QString paramName, QString value, QString &out, bool asTarget = false); //Out is the new value of the parameter, can be discarded
    bool setPythonSubparameter(int8_t cwId, QString paramName, QString subParamName, QString value, QString &out, bool asTarget = false); //Out is the new value of the subparameter, can be discarded
    bool downloadSamples(uint8_t cwId, size_t * size, void * out, bool asInt, size_t bufferSize);
    bool readFromTarget(uint8_t cwId, size_t * size, void * out, size_t bufferSize, QString func, unsigned long long addr = ULLONG_MAX);

    //bool getTracesFromShm(size_t &numTraces, size_t &traceSize, QList<double> &data);

public slots:
    static void handlePythonError(QProcess::ProcessError error);
    void checkForPythonState();
    void callbackPythonError();

protected:
    void _createPreInitParams();
    bool _validatePreInitParamsStructure(TConfigParam & params);

    const QString PLUGIN_ID = "TraceXpert.NewAE";

    //Methods for setup:
    bool setUpSHM(uint8_t cwId);
    bool setUpPythonProcess();
    bool testSHM(uint8_t cwId);
    bool autodetectDevices(QList<std::pair<QString, QString>> & devices);

    bool getDataFromShm(size_t &size, QString &data, uint8_t cwId, bool asTarget = false);
    bool getDataFromShm(size_t * size, void * data, uint8_t cwId, size_t bufferSize, bool asTarget = false);
    bool runPythonFunctionAndGetStringOutputHelper(int8_t cwId, const char* data, size_t len_in, size_t &dataLen, QString &out, bool asTarget = false);

    //In this block, CW is super important
    void packagePythonFunction(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out, bool asTarget = false);
    void packagePythonOnAnObjectFunction(uint8_t cwId, QString ObjectName, QString functionName, uint8_t numParams, QList<QString> params, QString &out, bool asTarget = false);
    void packagePythonParam(uint8_t cwId, QString paramName, QString value, QString &out, bool asTarget = false);
    void packagePythonSubparam(uint8_t cwId, QString paramName, QString subParamName, QString value, QString &out, bool asTarget = false);

    uint8_t numDevices; //This counts the number of **seen** devices, not the number of connected devices. Use m_scopes.lenght() for that
    uint8_t numActiveDevices;
    bool pythonReady[NO_CW_ID + 1];
    bool pythonError[NO_CW_ID + 1];
    bool pythonTargetReady[NO_CW_ID + 1];
    bool pythonTargetError[NO_CW_ID + 1];
    //bool deviceWaitingForRead;
    //uint8_t waitingForReadDeviceId;
    uint8_t lastCWActive;
    QString pythonPath;

    QList<TIODevice *> m_ports;
    QList<TScope *> m_scopes;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;

    QMap<uint8_t, QSharedMemory *> shmMap;
    QMap<uint8_t, QSharedMemory *> targetShmMap;

    //QSharedMemory shm;
    QProcess *pythonProcess;

    QString shmKey = PLUGIN_ID + "shm2";
    size_t shmSize;
    size_t targetShmSize;

    QMutex pythonProcessMutex;
    //QString pythonProcessStdOutData;
};


#endif // NEWAE_H
