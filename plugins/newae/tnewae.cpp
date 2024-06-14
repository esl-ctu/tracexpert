#include "tnewae.h"
//Next:
//sériovka k targetu
//víc cw najednou (fronta?, víc pyth. vláken?)
//2. co když se uživatel pokusí předidat stejné zařízení/cw dvakrát?
//cesta k .py souboru (jako preinitparam - ano - předkodovat?)



TNewae::TNewae(): m_ports(), m_preInitParams(), m_postInitParams() {
    m_preInitParams  = TConfigParam("NewAE pre-init configuration", "", TConfigParam::TType::TDummy, "");
    _createPreInitParams();

    numDevices = 0;
    for (int i = 0; i <= NO_CW_ID; ++i){
        pythonReady[i] = false;
        pythonError[i] = false;
        pythonTargetReady[i] = false;
        pythonTargetError[i] = false;
    }

    //deviceWaitingForRead = false;
    //waitingForReadDeviceId = NO_CW_ID;
    pythonPath = "";
    m_initialized = false;
    numActiveDevices = 0;
    //pythonProcessStdOutData = "";
}

TnewaeScope * TNewae::getCWScopeObjectById(uint8_t id){
    for (int i = 0; i < m_scopes.length(); ++i) {
        TnewaeScope * sc = (TnewaeScope *) m_scopes.at(i);
        uint8_t scId = sc->getId();
        if (scId == id) {
            return sc;
        }
    }
    return NULL;
}

void TNewae::_createPreInitParams(){
    m_preInitParams.addSubParam(TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect available NewAE devices", false));

    TConfigParam tmp = TConfigParam("Shared memory size", "1024", TConfigParam::TType::TInt, "Size of the memory shared between the python NewAE libraries and TraceXpert. \
                                             In kilobytes. Keep in mind that all numbers are transmitted as strings and as decimals. One int might need up to 11 bytes of SHM. \
                                             Traces from scope stay doubles.", false);
    m_preInitParams.addSubParam(tmp);

    TConfigParam tmp2 = TConfigParam("Shared memory size for target", "4", TConfigParam::TType::TInt, "Size of the memory shared between the python NewAE libraries and TraceXpert when communicating with a target. \
                                             In kilobytes.", false);

    m_preInitParams.addSubParam(tmp2);

    m_preInitParams.addSubParam(TConfigParam("Path to python executable (3.11 or newer)", QString(""), TConfigParam::TType::TString,
                                             "Path at which the python executable is located. At least python 3.11 is needed. \
                                             Leave blank to use python that is already installed and can be found in PATH. QT for python must also be installed",
                                             false));
}

bool TNewae::_validatePreInitParamsStructure(TConfigParam & params){
    bool iok;

    TConfigParam * par = params.getSubParamByName("Path to python executable (3.11 or newer)", &iok);
    if(!iok) return false;

    QString path = par->getValue();
    if (path == "") {
        pythonPath = "python";
    } else if (QFile::exists(path)) {
        pythonPath = path;
    } else {
        params.setState(TConfigParam::TState::TError, "Wrong structure of the pre-init params for NewAE plugin/component.");
        return false;
    }

    auto shmprm = m_preInitParams.getSubParamByName("Shared memory size", &iok);
    auto shmTargetPrm = m_preInitParams.getSubParamByName("Shared memory size for target", &iok);
    if(!iok) return false;

    shmSize = 1024 * shmprm->getValue().toInt();
    targetShmSize = 1024 * shmTargetPrm->getValue().toInt();

    if (shmSize == 0 || targetShmSize == 0) {
        params.setState(TConfigParam::TState::TError, "Wrong structure of the pre-init params for NewAE plugin/component.");
        return false;
    }

    return true;
}

TNewae::~TNewae() {
    (*this).TNewae::deInit();
}

QString TNewae::getName() const {
    return QString("NewAE");
}

QString TNewae::getInfo() const {
    return QString("Provides access to NewAE devices.");
}

bool TNewae::canAddIODevice(){
    return true;
}

bool TNewae::canAddScope(){
    return true;
}

TConfigParam TNewae::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TNewae::setPreInitParams(TConfigParam params) {
    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
        return m_preInitParams;
    }

    if(!_validatePreInitParamsStructure(params)) {
        qCritical("Wrong structure of the pre-init params for TNewAE. If you provided a path to the python executable, please check whether the file exists.");
        return m_preInitParams;
    }

    m_preInitParams = params;
    return m_preInitParams;
}

void TNewae::handlePythonError(QProcess::ProcessError error){
    switch (error){
    case QProcess::FailedToStart:
        qCritical("Python process error: Python process failed to start. Restart the program and make sure you have python installed.");
        break;
    case QProcess::Crashed:
        qCritical("Python process error: Python process crashed. Restart the program.");
        break;
    case QProcess::Timedout:
        qWarning("Python process error: An operation timed out and no more info is available. This may or may not be recoverable.");
        break;
    case QProcess::WriteError:
        qWarning("Python process error: Write error. This may or may not be recoverable.");
        break;
    case QProcess::ReadError:
        qWarning("Python process error: Read error. This may or may not be recoverable.");
        break;
    case QProcess::UnknownError:
        qWarning("Python process error: Unknown error. Please prepare a hammer and call Chuck Norris. Or restart the program.");
        break;
    }
}

bool TNewae::setUpSHM(uint8_t cwId){
    char cwIdShmKey[4];
    sprintf(cwIdShmKey, "%03d", cwId);

    QSharedMemory * cwSHM = new QSharedMemory();
    QSharedMemory * targetSHM = new QSharedMemory();
    QString wholeShmKey = shmKey + QString(cwIdShmKey);
    QString wholeTargetShmKey = wholeShmKey + QString("t");
    cwSHM->setKey(wholeShmKey);
    targetSHM->setKey(wholeTargetShmKey);

    bool succ = cwSHM->create(shmSize); //this also attaches the segment on success
    if (!succ && cwSHM->error() == QSharedMemory::AlreadyExists){
        cwSHM->attach();
    } else if (!succ) {
        qCritical("Failed to set up shared memory in C++.");
        return false;
    }

    succ = targetSHM->create(targetShmSize);
    if (!succ && targetSHM->error() == QSharedMemory::AlreadyExists){
        targetSHM->attach();
    } else if (!succ) {
        qCritical("Failed to set up shared memory in C++.");
        return false;
    }

    shmMap.insert(cwId, cwSHM);
    targetShmMap.insert(cwId, targetSHM);

    //Finish setting up the SHM in Python
    QString toSend;
    QList<QString> params;
    packageDataForPython(cwId, "SMSET:" + QString::number(shmSize), 0, params, toSend);
    succ = writeToPython(cwId, toSend);
    if (!succ){
        return false;
    }
    waitForPythonDone(cwId);

    if (pythonError[cwId]) {
        qCritical("Failed to set up scope shared memory in Python.");
        return false;
    }

    params.clear();
    packageDataForPython(cwId, "T-SMSET:" + QString::number(targetShmSize), 0, params, toSend);
    succ = writeToPython(cwId, toSend, true);
    if (!succ){
        return false;
    }
    waitForPythonTargetDone(cwId);

    if (pythonTargetError[cwId]) {
        qCritical("Failed to set up target shared memory in Python.");
        return false;
    }

    return true;
}

bool TNewae::setUpPythonProcess(){
    QString runDir(QCoreApplication::instance()->applicationDirPath());
    QString program = pythonPath;

    QStringList arguments;
    arguments << runDir + "/executable.py";

    pythonProcess = new QProcess;
    pythonProcess->setProcessChannelMode(QProcess::SeparateChannels);
    pythonProcess->setProgram(program);
    pythonProcess->setArguments(arguments);
    pythonProcess->start();
    pythonProcess->setReadChannel(QProcess::StandardOutput);
    QObject::connect(pythonProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this,  SLOT(handlePythonError(QProcess::ProcessError)));
    QObject::connect(pythonProcess, SIGNAL(readyReadStandardOutput()), this,  SLOT(checkForPythonState()));
    QObject::connect(pythonProcess, SIGNAL(readyReadStandardError()), this,  SLOT(callbackPythonError()));
    bool succ = pythonProcess->waitForStarted(PROCESS_WAIT_MSCECS); //wait max 30 seconds
    if (!succ){
        qCritical("Failed to start the python component. Did you provide a path to the correct executable? Or do you have python3 installed and symlinked as \"python\"?");
        return false;
    }

    succ = waitForPythonDone(NO_CW_ID);

    if (!succ){
        qCritical("The python component does not communicate. It will be killed.");
        switch (pythonProcess->state()){
        case QProcess::NotRunning:
            qCritical("%s", (("Error state: Not running " + pythonProcess->errorString())).toLocal8Bit().constData());
            break;
        case QProcess::Running:
            qCritical("%s", (("Error state: Running " + pythonProcess->errorString())).toLocal8Bit().constData());
            break;
        case QProcess::Starting:
            qCritical("%s", (("Error state: Starting " + pythonProcess->errorString())).toLocal8Bit().constData());
            break;
        }
        pythonProcess->kill();
        return false;
    }

    return true;
}

bool TNewae::testSHM(uint8_t cwId) {
    quint32 tmpval = QRandomGenerator::global()->generate();
    QString tmpstr = QString::number(tmpval);
    QString toSend;
    QList<QString> params;

    //Scope
    packageDataForPython(cwId, "SMTEST:" + tmpstr, 0, params, toSend);
    bool succ = writeToPython(cwId, toSend);

    if (!succ){
        qCritical("Failed to send data to Python when setting up the shared memory (scope).");
        return false;
    }

    succ = waitForPythonDone(cwId);
    succ &= !pythonError[cwId];
    if (!succ){
        qCritical("Python did not respond to SHM read request (scope).");
        return false;
    }

    size_t dataLen;
    QString data = "";
    succ = getDataFromShm(dataLen, data, cwId);
    if (!succ) qCritical("Error reading from shared memory (scope)");
    if (!dataLen) qCritical("No data from shared memory (scope)");
    succ &= data.contains(tmpstr);
    if (!succ){
        qCritical("%s", (("Failed to test the shared memory that was already set up (scope). Do you have Qt for Python installed? "
                          "If you do, please reboot your computer. SM data: " + data)).toLocal8Bit().constData());

        return false;
    }

    //Target
    tmpval = QRandomGenerator::global()->generate();
    QString tmpstrTarget = QString::number(tmpval);
    packageDataForPython(cwId, "T-SMTEST:" + tmpstrTarget, 0, params, toSend);
    succ = writeToPython(cwId, toSend, true);

    if (!succ){
        qCritical("Failed to send data to Python when setting up the shared memory (target).");
        return false;
    }

    succ = waitForPythonTargetDone(cwId);
    succ &= !pythonTargetError[cwId];
    if (!succ){
        qCritical("Python did not respond to SHM read request (target).");
        return false;
    }

    dataLen = 0;
    data = "";
    succ = getDataFromShm(dataLen, data, cwId, true);
    if (!succ) qCritical("Error reading from shared memory (target)");
    if (!dataLen) qCritical("No data from shared memory (target)");
    succ &= data.contains(tmpstrTarget);
    if (!succ){
        qCritical("%s", (("Failed to test the shared memory that was already set up (target). Do you have Qt for Python installed? "
                          "If you do, please reboot your computer. SM data: " + data)).toLocal8Bit().constData());

        return false;
    }

    return true;
}

bool TNewae::autodetectDevices(QList<std::pair<QString, QString>> & devices) {
        //Send data to python
        QString toSend;
        QList<QString> params;
        packageDataForPython(NO_CW_ID, "DETECT_DEVICES", 0, params, toSend);
        bool succ = writeToPython(NO_CW_ID, toSend);
        if (!succ){
            qWarning("Failed to send the DETECT DEVICES command to python.");
            return false;
        }

        //Read data from pyton
        size_t dataLen;
        QString data;
        succ &= waitForPythonDone(NO_CW_ID);
        succ &= !pythonError[NO_CW_ID];
        if (!succ){
            qWarning("Failed to receive response for the DETECT DEVICES command or received an invalid one.");
            return false;
        }
        getDataFromShm(dataLen, data, NO_CW_ID);

        //Parse the devices
        size_t i = 0;
        while (i != dataLen){
            QString name;
            QString sn;

            //Fill the name
            while((i != dataLen) &&
                   (data.at(i) != fieldSeparator)){
                name.append(data.at(i));
                i++;
            }

            //Eat the separator
            if ((i != dataLen) &&
                (data.at(i) == fieldSeparator)){
                ++i;
            } else {
                qWarning("A device was incompletely defined. All loaded devices are invalid.");
                return false;
            }

            //Fill the sn
            while((i != dataLen) &&
                   (data.at(i) != lineSeparator)){
                sn.append(data.at(i));
                i++;
            }

            //Is there a separator? Eat it
            if ((i != dataLen) &&
                (data.at(i) == lineSeparator)){
                ++i;
            }

            //Insert the device into the list for allocation
            devices.append(std::make_pair(name, sn));
        }
    return true;
}

bool TNewae::setUpAndTestSHM(uint8_t cwId) {
    //Create and attach the memory that's shared between C++ and the python process
    bool succ = setUpSHM(cwId);
    if(!succ) {
        return false;
    }

    //Wait handled in setUpSHM()

    //Test shared memory
    succ = testSHM(cwId);
    if(!succ) {
        return false;
    }

    return true;
}


void TNewae::init(bool *ok) {
    bool succ;
    bool autodetect = m_preInitParams.getSubParamByName("Auto-detect")->getValue() == "true";

    QSharedMemory * tmp = new QSharedMemory();
    shmMap.insert(NO_CW_ID, tmp);

    succ = _validatePreInitParamsStructure(m_preInitParams);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    //Create and run the python process
    succ = setUpPythonProcess();
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    succ = setUpAndTestSHM(NO_CW_ID);

    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    if(autodetect){
        //Auto detect devices
        QList<std::pair<QString, QString>> devices;
        succ = autodetectDevices(devices);
        if(!succ) {
            qCritical("No available devices autodetected. Please plug in your device (or make sure an another process is not using it) and re-initialize the component.");
            if(ok != nullptr) *ok = true;
            return;
        }

        //Append available devices to m_scopes/m_devices
        for(size_t i = 0; i < devices.size(); ++i) {
            addScopeAutomatically(devices.at(i).first, devices.at(i).second, &succ);
            if(!succ) {
                if(ok != nullptr) *ok = false;
                return;
            }
            addIODeviceAutomatically(devices.at(i).first, devices.at(i).second, &succ);
            if(!succ) {
                if(ok != nullptr) *ok = false;
                return;
            }
        }

        if (!numDevices){
            qCritical("No available devices autodetected. Please plug in your device (or make sure an another process is not using it) and re-initialize the component.");
        }
    }

    if(ok != nullptr) *ok = true;
    m_initialized = true;
}

void TNewae::deInit(bool *ok) {
    m_initialized = false;
    qDeleteAll(m_ports.begin(), m_ports.end());
    qDeleteAll(m_scopes.begin(), m_scopes.end());
    m_ports.clear();
    m_scopes.clear();
    if(ok != nullptr) *ok = true;

    if(m_initialized) {

        bool succ;

        //Stop python
        pythonProcess->write("HALT");
        pythonProcess->waitForBytesWritten();
        pythonProcess->closeWriteChannel();

        succ = pythonProcess->waitForFinished(PROCESS_WAIT_MSCECS);
        if (!succ){
            qWarning("Python component for NewAE devices did not exit gracefully. Killing...");
            pythonProcess->kill();
        }

    }

    pythonReady[NO_CW_ID] = false;

    //Detach nad delete shm
    qDeleteAll(shmMap);
    qDeleteAll(targetShmMap);
    shmMap.clear();
    targetShmMap.clear();

}

TConfigParam TNewae::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TNewae::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TNewae::addIODevice(QString name, QString info, bool *ok) {//TODO!!!!!
    /*TnewaeDevice * port = new TnewaeDevice(name, info, this, true);
    m_ports.append(port);
    return port;*/

    if(ok != nullptr) *ok = false;
    qWarning("Use automatic discovery");
    return NULL;
}

TIODevice * TNewae::addIODeviceAutomatically(QString name, QString info, bool *ok) {
    //Check if the port exists
    for (int i = 0; i < m_ports.length(); ++i){
        TnewaeDevice * port = (TnewaeDevice *) m_ports.at(i);
        QString portSn = port->getDeviceSn();
        if (portSn == info) {
            //Don't do anything, port already exists
            //No need to throw a warning
            if(ok != nullptr) *ok = true;
            return port;
        }
    }

    TnewaeDevice * port = new TnewaeDevice(name, info, this, false);
    m_ports.append(port);
    return port;
}

TScope * TNewae::addScope(QString name, QString info, bool *ok) {
    /*if (numDevices + 1 != NO_CW_ID) {
        TnewaeScope * sc;
        sc = new TnewaeScope(name, info, numDevices, this, true);
        m_scopes.append(sc);
        bool succ = setUpAndTestSHM(numDevices);
        numDevices++;
        if(ok != nullptr) *ok = succ;
        return sc;
    } else {
        qCritical("Number of available Chipwhisperer slots exceeded. Please de-init and re-init the plugin/component to continue.");
        if(ok != nullptr) *ok = false;
        return NULL;
    }*/
    if(ok != nullptr) *ok = false;
    qWarning("Use automatic discovery");
    return NULL;
}

TScope * TNewae::addScopeAutomatically(QString name, QString info, bool *ok) {
    //Check if the scope exists
    for (int i = 0; i < m_scopes.length(); ++i){
        TnewaeScope * sc = (TnewaeScope *) m_scopes.at(i);
        QString scSn = sc->getScopeSn();
        if (scSn == info) {
            //Don't do anything, scope already exists
            //No need to throw a warning
            if(ok != nullptr) *ok = true;
            return sc;
        }
    }

    if (numDevices + 1 != NO_CW_ID) {
        TnewaeScope * sc;
        sc = new TnewaeScope(name, info, numDevices, this, false);
        m_scopes.append(sc);
        pythonReady[numDevices] = true;
        pythonError[numDevices] = false;
        pythonTargetError[numDevices] = false;
        pythonTargetReady[numDevices] = true;
        bool succ = setUpAndTestSHM(numDevices);
        numDevices++;
        if(ok != nullptr) *ok = succ;
        return sc;
    } else {
        qCritical("Number of available Chipwhisperer slots exceeded. Please de-init and re-init the plugin/component to continue.");
        if(ok != nullptr) *ok = false;
        return NULL;
    }
}

QList<TIODevice *> TNewae::getIODevices() {
    return m_ports;
}

QList<TScope *> TNewae::getScopes() {
    return m_scopes;
}

void TNewae::packageDataForPython(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString & out){
    out = "";
    QTextStream(&out) <<  QString::number(cwId).rightJustified(3, '0');
    QTextStream(&out) << fieldSeparator << functionName;
    for (int i = 0; i < numParams; ++i){
        QTextStream(&out) << fieldSeparator << params.at(i);
    }
    QTextStream(&out) << lineSeparator;
}

void TNewae::packagePythonFunction(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out, bool asTarget /*= false*/){
    QString newFunctionName;
    if (asTarget)
        newFunctionName = "T-FUNC-" + functionName;
    else
        newFunctionName = "FUNC-" + functionName;
    packageDataForPython(cwId, newFunctionName, numParams, params, out);
}

void TNewae::packagePythonOnAnObjectFunctionWithNoParams(uint8_t cwId, QString ObjectName, QString functionName, QString &out){
    QString newFunctionName = "FUNO-" + ObjectName;
    QList<QString> params;
    params.append(functionName);
    packageDataForPython(cwId, newFunctionName, 1, params, out);
}

void TNewae::packagePythonParam(uint8_t cwId, QString paramName, QString value, QString &out, bool asTarget /*= false*/){
    QString newParamName;
    if (asTarget)
        newParamName = "T-PARA-" + paramName;
    else
        newParamName = "PARA-" + paramName;
    QList<QString> params;
    if (value == "") {
        packageDataForPython(cwId, newParamName, 0, params, out);
    } else {
        params.append(value);
        packageDataForPython(cwId, newParamName, 1, params, out);
    }
}

void TNewae::packagePythonSubparam(uint8_t cwId, QString paramName, QString subParamName, QString value, QString &out){
    QString newParamName = "SPAR-" + paramName;
    QList<QString> params;
    params.append(subParamName);
    if (value == "") {
        packageDataForPython(cwId, newParamName, 1, params, out);
    } else {
        params.append(value);
        packageDataForPython(cwId, newParamName, 2, params, out);
    }
}

bool TNewae::runPythonFunctionAndGetStringOutput(int8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, size_t &dataLen, QString &out, bool asTarget /*= false*/){
    QString toSend;
    bool succ;

    packagePythonFunction(cwId, functionName, numParams, params, toSend, asTarget);
    succ = writeToPython(cwId, toSend, asTarget);
    if(!succ) {
        return false;
    }

    if (asTarget){
        succ &= waitForPythonTargetDone(cwId);
        if(!succ || pythonTargetError[cwId]) {
            return false;
        }
    } else {
        succ &= waitForPythonDone(cwId);
        if(!succ || pythonError[cwId]) {
            return false;
        }
    }


    succ = getDataFromShm(dataLen, out, cwId, asTarget);
    if (!succ) {
        qCritical("Error reading from shared memory");
        return false;
    }

    if (!dataLen) {
        qCritical("No data from shared memory");
        return false;
    }

    return true;
}

bool TNewae::readFromTarget(uint8_t cwId, size_t * size, void * out, size_t bufferSize) {
    QString toSend;
    bool succ;
    QList<QString> params;

    packagePythonFunction(cwId, "read", 0, params , toSend, true);
    succ = writeToPython(cwId, toSend, true);
    if(!succ) {
        qDebug("Error sending the get_last_trace command.");
        return false;
    }

    succ &= waitForPythonTargetDone(cwId);
    if(!succ || pythonTargetError[cwId]) {
        return false;
    }

    succ = getDataFromShm(size, out, cwId, bufferSize, true);
    if (!succ) {
        qCritical("Error reading from shared memory");
        return false;
    }

    if (size == 0) {
        qCritical("No data from shared memory");
        return false;
    }

    return true;
}

bool TNewae::downloadSamples(uint8_t cwId, size_t * size, void * out, bool asInt, size_t bufferSize){
    QString toSend;
    bool succ;
    QList<QString> params;
    if (asInt) {
        params.append("true");
    } else {
        params.append("false");
    }

    packagePythonFunction(cwId, "get_last_trace", 1, params , toSend);
    succ = writeToPython(cwId, toSend);
    if(!succ) {
        qDebug("Error sending the get_last_trace command.");
        return false;
    }

    succ &= waitForPythonDone(cwId);
    if(!succ || pythonError[cwId]) {
        return false;
    }

    succ = getDataFromShm(size, out, cwId, bufferSize);
    if (!succ) {
        qCritical("Error reading from shared memory");
        return false;
    }

    if (*size == 0) {
        qCritical("No data from shared memory");
        return false;
    }

    return true;
}

bool TNewae::runPythonFunctionOnAnObjectAndGetStringOutput(int8_t cwId, QString ObjectName, QString functionName, size_t &dataLen, QString &out){
    QString toSend;
    bool succ;

    packagePythonOnAnObjectFunctionWithNoParams(cwId, ObjectName, functionName, toSend);
    succ = writeToPython(cwId, toSend);
    if(!succ) {
        return false;
    }

    succ &= waitForPythonDone(cwId);
    if(!succ || pythonError[cwId]) {
        return false;
    }

    succ = getDataFromShm(dataLen, out, cwId);
    if (!succ) {
        qCritical("Error reading from shared memory");
        return false;
    }

    if (!dataLen) {
        qCritical("No data from shared memory");
        return false;
    }

    return true;
}

bool TNewae::getPythonParameter(int8_t cwId, QString paramName, QString &out, bool asTarget /*= false*/){
    QString tmp = "";
    return setPythonParameter(cwId, paramName, tmp, out, asTarget);
}

bool TNewae::getPythonSubparameter(int8_t cwId, QString paramName, QString subParamName, QString &out){
    QString tmp = "";
    return setPythonSubparameter(cwId, paramName, subParamName, tmp, out);
}

bool TNewae::setPythonParameter(int8_t cwId, QString paramName, QString value, QString &out, bool asTarget /*= false*/){
    QString toSend;
    bool succ;

    packagePythonParam(cwId, paramName, value, toSend, asTarget);
    succ = writeToPython(cwId, toSend, asTarget);

    if(!succ) {
        return false;
    }

    if (asTarget)
        succ &= waitForPythonTargetDone(cwId);
    else
        succ &= waitForPythonDone(cwId);

    if(!succ) {
        return false;
    }

    size_t dataLen;
    if (asTarget)
        succ = getDataFromShm(dataLen, out, cwId, true);
    else
        succ = getDataFromShm(dataLen, out, cwId);

    if (!succ) {
        qCritical("Error reading from shared memory");
        return false;
    }

    if (!dataLen) {
        qCritical("No data from shared memory");
        return false;
    }

    return true;
}

bool TNewae::setPythonSubparameter(int8_t cwId, QString paramName, QString subParamName, QString value, QString &out){
    QString toSend;
    bool succ;

    packagePythonSubparam(cwId, paramName, subParamName, value, toSend);
    succ = writeToPython(cwId, toSend);

    if(!succ) {
        return false;
    }

    succ &= waitForPythonDone(cwId);
    if(!succ) {
        return false;
    }

    size_t dataLen;
    succ = getDataFromShm(dataLen, out, cwId);
    if (!succ) {
        qCritical("Error reading from shared memory");
        return false;
    }

    if (!dataLen) {
        qCritical("No data from shared memory");
        return false;
    }

    return true;
}

bool TNewae::writeToPython(uint8_t cwId, const QString &data, bool asTarget /*= false*/, bool responseExpected/* = true*/, bool wait/* = true*/){
    if(asTarget) {
        waitForPythonTargetDone(cwId, 1000);

        if (!pythonTargetReady[cwId]){
            qDebug("Python not ready! (target)");
            return false;
        }
        pythonTargetReady[cwId] = false;
    } else {
        waitForPythonDone(cwId, 1000);

        if (!pythonReady[cwId]){
            qDebug("Python not ready!");
            return false;
        }
        pythonReady[cwId] = false;
    }

    wait = true; //!!!!!

    int succ;

    succ = pythonProcess->write(data.toLocal8Bit().constData());

    if (succ == -1){
        return false;
    }

    if (wait){
        succ = pythonProcess->waitForBytesWritten();
    }

    if (succ == -1){
        return false;
    }

    if (!responseExpected && asTarget){
        pythonTargetReady[cwId] = true;
    }

    if (!responseExpected && !asTarget){
        pythonReady[cwId] = true;
    }

    return true;
}

void TNewae::callbackPythonError() {
    QString data = pythonProcess->readAllStandardError();
    qWarning("%s", (("NewAE python component returned the following error (this might or might not be recoverable): " + data)).toLocal8Bit().constData());
}

void TNewae::checkForPythonState(){
    QString buff;

    pythonProcessStdOutMutex.lock();
    //pythonProcessStdOutData = pythonProcessStdOutData + pythonProcess->readAllStandardOutput();
    buff = pythonProcess->readAllStandardOutput();
    pythonProcessStdOutMutex.unlock();

    //najít DONE/STARTED/NOTCN/ERROR
    //vyzobat to z toho pro všechny dostupný CW

    int fromIndexDone = 0;
    int fromIndexStarted = 0;
    int fromIndexNotcn = 0;
    int fromIndexError = 0;

    while (true) {
        int indexDone = buff.indexOf("DONE", fromIndexDone);
        int indexStarted = buff.indexOf("STARTED", fromIndexStarted);
        int indexNotcn = buff.indexOf("NOTCN", fromIndexNotcn);
        int indexError = buff.indexOf("ERROR", fromIndexError);

        if (indexDone != -1){
            QString type = buff.sliced(indexDone + 5, 2);
            QString id = buff.sliced(indexDone + 8, 3);
            uint8_t idUint = id.toUShort();

            if (type == "IO"){
                pythonTargetReady[idUint] = true;
                pythonTargetError[idUint] = false;
            }

            if (type == "SC") {
                pythonReady[idUint] = true;
                pythonError[idUint] = false;
            }

            fromIndexDone = indexDone + 11;
        }

        if (indexStarted != -1){
            pythonReady[NO_CW_ID] = true;
            pythonError[NO_CW_ID] = false;
            pythonTargetReady[NO_CW_ID] = true;
            pythonTargetError[NO_CW_ID] = false;

            fromIndexStarted = indexStarted + 7;
        }

        if (indexNotcn != -1){
            QString type = buff.sliced(indexDone + 6, 2);
            QString id = buff.sliced(indexDone + 9, 3);
            uint8_t idUint = id.toUShort();

            if (type == "IO"){
                pythonTargetReady[idUint] = true;
                pythonTargetError[idUint] = true;
            }

            if (type == "SC") {
                pythonError[idUint] = true;
                pythonReady[idUint] = true;
            }

            TnewaeScope * sc = getCWScopeObjectById(idUint);
            if (sc) sc->notConnectedError();

            fromIndexNotcn = indexNotcn + 12;
        }

        if (indexError != -1){
            QString type = buff.sliced(indexDone + 6, 2);
            QString id = buff.sliced(indexDone + 9, 3);
            type.truncate(2);
            id.truncate(3);
            uint8_t idUint = id.toUShort();
            if (type == "IO"){
                pythonTargetReady[idUint] = true;
                pythonTargetError[idUint] = true;
            }

            if (type == "SC") {
                pythonError[idUint] = true;
                pythonReady[idUint] = true;
            }

            fromIndexError = indexError + 12;
        }


        if (indexDone == -1 && indexStarted == -1 && indexNotcn == -1 && indexError == -1)
            break;
    }
}


bool TNewae::waitForPythonDone(uint8_t cwId, int timeout/* = 30000*/){
    for (int i = 0; i < timeout/50; ++i) {
        if (pythonReady[cwId]){

            break;
        }
        pythonProcess->waitForReadyRead(50);
    }

    return pythonReady[cwId];
}

bool TNewae::waitForPythonTargetDone(uint8_t cwId, int timeout/* = 30000*/){
    for (int i = 0; i < timeout/50; ++i) {
        if (pythonTargetReady[cwId]){
            break;
        }
        pythonProcess->waitForReadyRead(50);
    }

    return pythonTargetReady[cwId];
}

/*bool TNewae::getTracesFromShm(size_t &numTraces, size_t &traceSize, QList<double> &data){
    char* numTracesAddr;
    char* traceSizeAddr;
    bool succ, succ2;

    succ = shm.lock();

    char * shmData = (char *) (shm.data());
    numTracesAddr = shmData + SM_NUM_TRACES_ADDR;
    QString numTracesStr = "";
    for (int i = 0; i < ADDR_SIZE; ++i){
        numTracesStr += numTracesAddr[i];
    }
    numTraces = numTracesStr.toULongLong(&succ2, 16);
    succ &= succ2;

    traceSizeAddr = shmData + SM_TRACE_SIZE_ADDR;
    QString traceSizeStr = "";
    for (int i = 0; i < ADDR_SIZE; ++i){
        traceSizeStr += traceSizeAddr[i];
    }
    traceSize = traceSizeStr.toULongLong(&succ2, 16);
    succ &= succ2;

    double* traceData = (double*) shmData + SM_TRACES_ADDR;

    //Get data
    data.clear();
    try {
        data.reserve(traceSize * numTraces + 1);
    } catch (const std::bad_alloc& e) {
        qCritical("Unable to reserve enough memory to read from SHM. Wanted to reserve: %zu", traceSize * numTraces + 1);
        return false;
    }
    for (int i = 0; i < traceSize * numTraces; ++i){
        data.append(shmData[i]);
    }

    succ = succ & shm.unlock();
    return succ;
}*/

bool TNewae::getDataFromShm(size_t &size, QString &data, uint8_t cwId, bool asTarget /* = false */){
    char* dataLenAddr;
    bool succ, succ2;

    QSharedMemory * shm;
    if(asTarget)
        shm = targetShmMap.value(cwId, NULL);
    else
        shm = shmMap.value(cwId, NULL);

    if (shm == NULL)
        return false;

    succ = shm->lock();

    //Get data pointer and data size
    char * shmData = (char *) (shm->data());
    dataLenAddr = shmData + SM_SIZE_ADDR;
    QString sizeStr = "";
    for (int i = 0; i < ADDR_SIZE; ++i){
        sizeStr += dataLenAddr[i];
    }
    shmData += SM_DATA_ADDR;
    size = sizeStr.toULongLong(&succ2, 16);
    succ &= succ2;

    //Get data
    data = "";
    try {
        data.reserve(size + 1);
    } catch (const std::bad_alloc& e) {
        qCritical("Unable to reserve enough memory to read from SHM. Wanted to reserve: %zu", size);
        return false;
    }

    for(size_t i = 0; i < size; ++i){
        data.append(shmData[i]);
    }

    succ = succ & shm->unlock();

    return succ;
}

bool TNewae::getDataFromShm(size_t * size, void * data, uint8_t cwId, size_t bufferSize, bool asTarget /* = false */){
    char* dataLenAddr;
    bool succ, succ2;

    QSharedMemory * shm;
    if(asTarget)
        shm = targetShmMap.value(cwId, NULL);
    else
        shm = shmMap.value(cwId, NULL);

    if (shm == NULL)
        return false;

    succ = shm->lock();

    //Get data pointer and data size
    char * shmData = (char *) (shm->data());
    dataLenAddr = shmData + SM_SIZE_ADDR;
    QString sizeStr = "";
    for (int i = 0; i < ADDR_SIZE; ++i){
        sizeStr += dataLenAddr[i];
    }
    shmData += SM_DATA_ADDR;
    *size = sizeStr.toULongLong(&succ2, 16);
    succ &= succ2;

    if (*size > bufferSize)
        *size = bufferSize;

    memcpy(data, shmData, *size);

    succ = succ & shm->unlock();

    return succ;
}

