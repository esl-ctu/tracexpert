#include "tnewae.h"

TNewae::TNewae(): m_ports(), m_preInitParams(), m_postInitParams() {
   m_preInitParams  = TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect available NewAE devices", false);
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

void TNewae::init(bool *ok) {
    bool succ;

    //Create and run the python process
    QString program = "python3";
    QStringList arguments;
    arguments << "executable.py";

    pythonProcess = new QProcess;
    pythonProcess->setProcessChannelMode(QProcess::MergedChannels);
    pythonProcess->start(program, arguments);
    succ = pythonProcess->waitForStarted(30000); //wait max 30 seconds
    if (!succ){
        *ok = false;
        qWarning("Failed to start the python component.");
        return;
    }
    QByteArray data = pythonProcess->readAllStandardOutput();
    succ = data.contains("STARTED");
    if (!succ){
        *ok = false;
        qWarning("The python component does not communicate. It will be killed.");
        pythonProcess->kill();
        return;
    }

    //Create and attach the memory that's shared between C++ and the python process
    shm.setKey(shmKey);
    succ = shm.create(shmSize); //this also attaches the segment on success
    if (!succ && shm.error() == QSharedMemory::AlreadyExists){
        shm.attach();
    } else {
        *ok = false;
        qWarning("Failed to set up shared memory.");
        return;
    }

    //Auto detect devices
    if(m_preInitParams.getName() == "Auto-detect" && m_preInitParams.getValue() == "true") {
        //Send data to python
        QString toSend;
        QList<QString> params;
        packageDataForPython(-1, "DETECT_DEVICES", 0, params, toSend);
        pythonProcess->write(toSend.toLocal8Bit().constData());

        //Read data from pyton
        size_t dataLen;
        QList<uint8_t> data;
        getDataFromShm(dataLen, data);

        //Parse the devices
        QList<std::pair<QString, QString>> devices;
        size_t i = 0;
        while (i != dataLen){
            QString name;
            QString sn;

            //Fill the name
            while((i != dataLen) &&
                   (data.at(i) != lineSeparator)){
                name.append((char) data.at(i));
                ++i;
            }

            //Eat the separator
            if ((i != dataLen) &&
                (data.at(i) != lineSeparator)){
                ++i;
            } else {
                *ok = false;
                qWarning("A device was incompletely defined. All loaded devices are invalid.");
            }

            //Fill the sn
            while((i != dataLen) &&
                   (data.at(i) != lineSeparator)){
                sn.append((char) data.at(i));
                ++i;
            }

            //Is there a separator? Eat it
            if ((i != dataLen) &&
                (data.at(i) != lineSeparator)){
                ++i;
            }

            //Insert the device into the list for allocation
            devices.append(std::make_pair(name, sn));
        }

        //Append available devices to m_ports
        for(size_t i = 0; i < dataLen; ++i) {
            m_ports.append(new TnewaeDevice(devices.at(i).first, devices.at(i).second));
        }
    }
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

    succ = pythonProcess->waitForFinished(30000);
    if (!succ){
        qWarning("Python component for NewAE devices did not exit gracefully. Killing...");
        pythonProcess->kill();
    }

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
    m_ports.append(new TnewaeDevice(name, sn));
    if(ok != nullptr) *ok = true;
}

void TNewae::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
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
}

bool TNewae::getDataFromShm(size_t &size, QList<uint8_t> &data){
    size_t* dataLenAddr;
    bool succ;

    succ = shm.lock();

    //Get data pointer and data size
    uint8_t * shmData = (uint8_t *) (shm.data());
    dataLenAddr = (size_t *) (shmData + SM_SIZE_ADDR);
    size = (*dataLenAddr);
    shmData += SM_DATA_ADDR;

    //Get data
    data.reserve(size + 1);
    for(size_t i = 0; i < size; ++i){
        data.append(shmData[i]);
    }

    succ = succ & shm.unlock();

    return succ;
}
