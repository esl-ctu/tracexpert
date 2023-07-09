#include "tnewae.h"

//TODO Exposing io devices but need to expose scopes
//TODO handle line separators - test - waht did I mean?

//Next:
//TODO rozdíl mezi checkForPythonReady a waitForPythonDone??
//TODO vyřešit co když python vrátí fail
//TODO STDERR od Pythonu musí vyhodit QWarning

TNewae::TNewae(): m_ports(), m_preInitParams(), m_postInitParams() {
    m_preInitParams  = TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect available NewAE devices", false);
    numDevices = 0;
    pythonReady = false;
    deviceWaitingForRead = false;
    waitingForReadDeviceId = NO_CW_ID;
}

TNewae::~TNewae() {
    (*this).TNewae::deInit();
}

QString TNewae::getPluginName() const {
    return QString("NewAE");
}

QString TNewae::getPluginInfo() const {
    return QString("Provides access to NewAE devices.");
}


TConfigParam TNewae::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TNewae::setPreInitParams(TConfigParam params) {
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
    QString program = "python";

    QStringList arguments;
    arguments << runDir + "/executable.py";

    pythonProcess = new QProcess;
    pythonProcess->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    pythonProcess->setProgram(program);
    pythonProcess->setArguments(arguments);
    pythonProcess->start();
    pythonProcess->setReadChannel(QProcess::StandardOutput);
    QObject::connect(pythonProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this,  SLOT(handlePythonError(QProcess::ProcessError)));
    bool succ = pythonProcess->waitForStarted(PROCESS_WAIT_MSCECS); //wait max 30 seconds
    if (!succ){
        qCritical("Failed to start the python component. Do you have python3 installed and symlinked as \"python\"?");
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

    succ = waitForPythonDone(NO_CW_ID);
    if (!succ){
        qCritical("Python did not respond to SHM read request.");
        return false;
    }

    size_t dataLen;
    QString data = "";
    succ = getDataFromShm(dataLen, data);
    if (!succ) qCritical("no data from mem");
    succ &= data.contains(tmpstr);
    if (!succ){
        qCritical("Failed to test the shared memory that was already set up. Do you have Qt for Python installed? If you do, please reboot your computer.");
        return false;
    }

    return true;
}

bool TNewae::autodetectDevices(QList<std::pair<QString, QString>> & devices) {
    if(m_preInitParams.getName() == "Auto-detect" && m_preInitParams.getValue() == "true") {
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
        if (!succ){
            qWarning("Failed to receive response for the DETECT DEVICES command.");
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
    }
    return true;
}

void TNewae::init(bool *ok) {
    bool succ;

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

    pythonReady = true;

    //Test shared memory
    succ = testSHM();
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }


    //Auto detect devices
    QList<std::pair<QString, QString>> devices;
    succ = autodetectDevices(devices);
    if(!succ) {
        if(ok != nullptr) *ok = false;
        return;
    }

    //Append available devices to m_ports
    for(size_t i = 0; i < devices.size(); ++i) {
        if (numDevices + 1 != NO_CW_ID) {
            m_scopes.append(new TnewaeScope(devices.at(i).first, devices.at(i).second, numDevices, this));
            numDevices++;
        } else {
            qCritical("Number of available Chipwhisperer slots exceeded. Please de-init and re-init the plugin to continue.");
        }
    }

    if (!numDevices){
        qWarning("No devices autodetected.");
    }

    if(ok != nullptr) *ok = true;
}

void TNewae::deInit(bool *ok) {
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

void TNewae::addIODevice(QString name, QString sn, bool *ok) {
    m_ports.append(new TnewaeDevice(name, sn, numDevices));
    numDevices++;
    if(ok != nullptr) *ok = true;
}

void TNewae::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    //TODO!!
}

QList<TIODevice *> TNewae::getIODevices() {
    return m_ports;
}

QList<TScope *> TNewae::getScopes() {
    return QList<TScope *>();
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

bool TNewae::writeToPython(uint8_t cwId, const QString &data, bool responseExpected/* = true*/, bool wait/* = true*/){
    if (!pythonReady){
        return false;
    }
    pythonReady = false;
    wait = true; //!!!!!

    int succ;

    succ = pythonProcess->write(data.toLocal8Bit().constData());

    if (succ == -1){
        return false;
    }

    if (wait){
        succ = pythonProcess->waitForBytesWritten();
        pythonReady = true;
    }

    if (succ == -1){
        return false;
    }

    if (responseExpected){
        deviceWaitingForRead = true;
        waitingForReadDeviceId = cwId;
    }


    return true;
}

bool TNewae::checkForPythonReady(int wait /*= 30000*/){
    if (wait){
        pythonProcess->waitForReadyRead(wait);
    }
    QString buff;
    buff = pythonProcess->peek(6);
    return (buff.contains("DONE") || buff.contains("STARTED") || buff.contains("ERROR"));
}

bool TNewae::checkForPythonError(){
    QString buff;
    buff = pythonProcess->peek(6);
    if (buff.contains("ERROR")) {
        pythonProcess->readAllStandardOutput();
        pythonReady = true;
        deviceWaitingForRead = false;
        waitingForReadDeviceId = -1;
        return true;
    }

    return false;
}

//Call check for ready first!
bool TNewae::readFromPython(uint8_t cwId, QString &data, bool wait/* = true*/){
    if (!pythonReady || !deviceWaitingForRead || cwId != waitingForReadDeviceId){
        return false;
    }

    data = pythonProcess->readAllStandardOutput();
    deviceWaitingForRead = false;

    return true;
}

bool TNewae::waitForPythonDone(uint8_t cwId, int timeout/* = 30000*/){
    if (!(checkForPythonReady(timeout)))
        return false;

    QString buff;
    buff = pythonProcess->peek(6);
    if (buff.contains("DONE") ) {
        return readFromPython(cwId, buff);;
    }

    return false;
}

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
    }

    for(size_t i = 0; i < size; ++i){
        data.append(shmData[i]);
    }

    succ = succ & shm.unlock();

    return succ;
}
