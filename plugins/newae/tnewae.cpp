#include "tnewae.h"
//Next:
//sériovka k targetu
//počítadlo aktivních zařízení (netřeba?)
//1. podpora volání fcí nad objekty na straně pythonu (hotovo, nestestováno)
///potřebuju někde parametry? teď je c++ neumí
//2. cwBufferSize jako pre init param (hotovo, netestováno)
//3. kontrola post init params (hotovo, netestováno)
//4. fce TnewaeScope::getChannelsStatus() (hotovo, netestováno, zkontrolovat na příštím meetingu)
//5. test s cw
//6. vyřešit traces as int nebo ne (spíš ne?)
//7. přepsat parametry na enumy (nice to have)
//8. cesta k .py souboru
//9. run() nemá být blokující
//10.armuju scope? (hotovo, stačí to takhle?)
////Zkusit přímo v pythonu



TNewae::TNewae(): m_ports(), m_preInitParams(), m_postInitParams() {
    m_preInitParams  = TConfigParam("NewAE pre-init configuration", "", TConfigParam::TType::TDummy, "");
    _createPreInitParams();

    numDevices = 0;
    pythonReady = false;
    pythonError = false;
    deviceWaitingForRead = false;
    waitingForReadDeviceId = NO_CW_ID;
    pythonPath = "";
    m_initialized = false;
    numActiveDevices = 0;
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

    m_preInitParams.addSubParam(TConfigParam("Path to python executable", QString(""), TConfigParam::TType::TString,
                                             "Path at which the python executable is located. At least python 3.11 is needed. \
                                             Leve blank to use python that is already installed and can be found in PATH. QT for python must also be installed",
                                             false));
}

bool TNewae::_validatePreInitParamsStructure(TConfigParam & params){
    bool iok;

    TConfigParam * par = params.getSubParamByName("Path to python executable", &iok);
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

    auto tmp = m_preInitParams.getSubParamByName("Shared memory size", &iok);
    if(!iok) return false;

    shmSize = 1024 * tmp->getValue().toInt();

    if (shmSize == 0) {
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

bool TNewae::setUpSHM(){
    shm.setKey(shmKey);
    bool succ = shm.create(shmSize); //this also attaches the segment on success
    if (!succ && shm.error() == QSharedMemory::AlreadyExists){
        shm.attach();
    } else if (!succ) {
        qCritical("Failed to set up shared memory.");
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

    succ = pythonProcess->waitForReadyRead(PROCESS_WAIT_MSCECS);
    QString data = pythonProcess->readAllStandardOutput();
    succ &= data.contains("STARTED");

    if (!succ){
        qCritical("%s", (QString("The python component does not communicate. It will be killed. This is its output:") + QString(data)).toLocal8Bit().constData());
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

bool TNewae::testSHM() {
    quint32 tmpval = QRandomGenerator::global()->generate();
    QString tmpstr = QString::number(tmpval);
    bool succ = writeToPython(NO_CW_ID, "SMTEST:" + tmpstr + lineSeparator);
    if (!succ){
        qCritical("Failed to send data to Python when setting up the shared memory.");
        return false;
    }

    succ = waitForPythonDone(NO_CW_ID, true);
    succ &= !pythonError;
    if (!succ){
        qCritical("Python did not respond to SHM read request.");
        return false;
    }

    size_t dataLen;
    QString data = "";
    succ = getDataFromShm(dataLen, data);
    if (!succ) qCritical("Error reading from shared memory");
    if (!dataLen) qCritical("No data from shared memory");
    succ &= data.contains(tmpstr);
    if (!succ){
        qCritical("%s", (("Failed to test the shared memory that was already set up. Do you have Qt for Python installed? "
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
        succ &= waitForPythonDone(NO_CW_ID, true);
        succ &= !pythonError;
        if (!succ){
            qWarning("Failed to receive response for the DETECT DEVICES command or received an invalid one.");
            return false;
        }
        getDataFromShm(dataLen, data);

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

void TNewae::init(bool *ok) {
    bool succ;
    bool autodetect = m_preInitParams.getSubParamByName("Auto-detect")->getValue() == "true";

    succ = _validatePreInitParamsStructure(m_preInitParams);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    //Create and attach the memory that's shared between C++ and the python process
    succ = setUpSHM();
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

    //Finish setting up the SHM
    succ = writeToPython(NO_CW_ID, "SMSET:" + QString::number(shmSize) + lineSeparator);
    if (!succ){
        if(ok != nullptr) *ok = false;
        return;
    }

    succ = waitForPythonDone(NO_CW_ID, true);
    succ &= !pythonError;
    if (!succ){
        if(ok != nullptr) *ok = false;
        return;
    }

    //Test shared memory
    succ = testSHM();
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    if(autodetect){
        //Auto detect devices
        QList<std::pair<QString, QString>> devices;
        succ = autodetectDevices(devices);
        if(!succ) {
            if(ok != nullptr) *ok = false;
            return;
        }

        //Append available devices to m_scopes
        for(size_t i = 0; i < devices.size(); ++i) {
            addScopeAutomatically(devices.at(i).first, devices.at(i).second, &succ);
            if(!succ) {
                if(ok != nullptr) *ok = false;
                return;
            }
        }

        if (!numDevices){
            qWarning("No devices autodetected.");
        }
    }

    if(ok != nullptr) *ok = true;
    m_initialized = true;
}

void TNewae::deInit(bool *ok) {
    m_initialized = false;
    qDeleteAll(m_ports.begin(), m_ports.end());
    m_ports.clear();
    if(ok != nullptr) *ok = true;

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

    pythonReady = false;

    //Detach shm
    succ = shm.detach();
    if (!succ){
        if(ok != nullptr) *ok = false;
    }
}

TConfigParam TNewae::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TNewae::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TNewae::addIODevice(QString name, QString info, bool *ok) {
    //m_ports.append(new TnewaeDevice(name, info, numDevices));
    //numDevices++;
    if(ok != nullptr) *ok = true;
    //TODO
}

TScope * TNewae::addScope(QString name, QString info, bool *ok) {
    if (numDevices + 1 != NO_CW_ID) {
        TnewaeScope * sc;
        sc = new TnewaeScope(name, info, numDevices, this, true);
        m_scopes.append(sc);
        numDevices++;
        if(ok != nullptr) *ok = true;
        return sc;
    } else {
        qCritical("Number of available Chipwhisperer slots exceeded. Please de-init and re-init the plugin/component to continue.");
        if(ok != nullptr) *ok = false;
        return NULL;
    }
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
        numDevices++;
        if(ok != nullptr) *ok = true;
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

void TNewae::packagePythonFunction(uint8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, QString &out){
    QString newFunctionName = "FUNC-" + functionName;
    packageDataForPython(cwId, newFunctionName, numParams, params, out);
}

void TNewae::packagePythonOnAnObjectFunctionWithNoParams(uint8_t cwId, QString ObjectName, QString functionName, QString &out){
    QString newFunctionName = "FUNO-" + ObjectName;
    QList<QString> params;
    params.append(functionName);
    packageDataForPython(cwId, newFunctionName, 1, params, out);
}

void TNewae::packagePythonParam(uint8_t cwId, QString paramName, QString value, QString &out){
    QString newParamName = "PARA-" + paramName;
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

bool TNewae::runPythonFunctionAndGetStringOutput(int8_t cwId, QString functionName, uint8_t numParams, QList<QString> params, size_t &dataLen, QString &out){
    QString toSend;
    bool succ;

    packagePythonFunction(cwId, functionName, numParams,params , toSend);
    succ = writeToPython(cwId, toSend);
    if(!succ) {
        return false;
    }

    succ &= waitForPythonDone(cwId, true);
    if(!succ || pythonError) {
        return false;
    }

    succ = getDataFromShm(dataLen, out);
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

bool TNewae::runPythonFunctionOnAnObjectAndGetStringOutput(int8_t cwId, QString ObjectName, QString functionName, size_t &dataLen, QString &out){
    QString toSend;
    bool succ;

    packagePythonOnAnObjectFunctionWithNoParams(cwId, ObjectName, functionName, toSend);
    succ = writeToPython(cwId, toSend);
    if(!succ) {
        return false;
    }

    succ &= waitForPythonDone(cwId, true);
    if(!succ || pythonError) {
        return false;
    }

    succ = getDataFromShm(dataLen, out);
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

bool TNewae::getPythonParameter(int8_t cwId, QString paramName, QString &out){
    QString tmp = "";
    return setPythonParameter(cwId, paramName, tmp, out);
}

bool TNewae::getPythonSubparameter(int8_t cwId, QString paramName, QString subParamName, QString &out){
    QString tmp = "";
    return setPythonSubparameter(cwId, paramName, subParamName, tmp, out);
}

bool TNewae::setPythonParameter(int8_t cwId, QString paramName, QString value, QString &out){
    QString toSend;
    bool succ;

    packagePythonParam(cwId, paramName, value, toSend);
    succ = writeToPython(cwId, toSend);

    if(!succ) {
        return false;
    }

    succ &= waitForPythonDone(cwId, true);
    if(!succ) {
        return false;
    }

    size_t dataLen;
    succ = getDataFromShm(dataLen, out);
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

    succ &= waitForPythonDone(cwId, true);
    if(!succ) {
        return false;
    }

    size_t dataLen;
    succ = getDataFromShm(dataLen, out);
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

bool TNewae::writeToPython(uint8_t cwId, const QString &data, bool responseExpected/* = true*/, bool wait/* = true*/){
    if (!pythonReady){
        return false;
    }
    pythonReady = false;
    wait = true; //!!!!!
    lastCWActive = cwId;

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

    if (responseExpected){
        deviceWaitingForRead = true;
        waitingForReadDeviceId = cwId;
    } else {
        pythonReady = true;
    }

    return true;
}

void TNewae::callbackPythonError() {
    QString data = pythonProcess->readAllStandardError();
    qWarning("%s", (("NewAE python component returned the following error (this might or might not be recoverable): " + data)).toLocal8Bit().constData());
}

void TNewae::checkForPythonState(){
    QString buff;
    buff = pythonProcess->peek(1024*1024);

    if (pythonReady)
        return;

    if (buff.contains("DONE")){
        pythonReady = true;
        pythonError = false;
    } else if (buff.contains("STARTED")){
        pythonReady = true;
        pythonError = false;
    }else if (buff.contains("NOTCN")){
        pythonReady = true;
        pythonError = true;
        TnewaeScope * sc = getCWScopeObjectById(lastCWActive);
        if (sc) sc->notConnectedError();
    }else if (buff.contains("ERROR")){
        pythonReady = true;
        pythonError = true;

        //pythonProcess->readAllStandardOutput();
        //deviceWaitingForRead = false;
        //waitingForReadDeviceId = -1;
    } else {
        pythonReady = false;
        pythonError = false;
    }
}

bool TNewae::readFromPython(uint8_t cwId, QString &data, bool wait/* = true*/){
    if (!pythonReady || !deviceWaitingForRead || cwId != waitingForReadDeviceId){
        return false;
    }

    data = pythonProcess->readAllStandardOutput();
    deviceWaitingForRead = false;

    return true;
}

bool TNewae::waitForPythonDone(uint8_t cwId, bool discardOutput, int timeout/* = 30000*/){
    for (int i = 0; i < 15; ++i) {
        if (pythonReady){
            break;
        }
        pythonProcess->waitForReadyRead(timeout/15);
    }

    if (!pythonReady || !deviceWaitingForRead || cwId != waitingForReadDeviceId){
        return false;
    }

    if (discardOutput) {
        QString tmp;
        readFromPython(cwId, tmp);
    }

    return true;
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

bool TNewae::getDataFromShm(size_t &size, QString &data){
    char* dataLenAddr;
    bool succ, succ2;

    succ = shm.lock();

    //Get data pointer and data size
    char * shmData = (char *) (shm.data());
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

    succ = succ & shm.unlock();

    return succ;
}
