#include "tnewaedevice.h"

TnewaeDevice::TnewaeDevice(QString & name, QString & info){

}


TnewaeDevice::~TnewaeDevice(){

}

QString TnewaeDevice::getIODeviceName() const{
    return m_name;
}

QString TnewaeDevice::getIODeviceInfo() const{
    return m_info;
}

TConfigParam TnewaeDevice::getPreInitParams() const{
    return m_preInitParams;
}

TConfigParam TnewaeDevice::setPreInitParams(TConfigParam params){
    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

void TnewaeDevice::init(bool *ok/* = nullptr*/){
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

    //TODO intialize devices
}

void TnewaeDevice::deInit(bool *ok/* = nullptr*/){
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
        *ok = false;
    }
}

TConfigParam TnewaeDevice::getPostInitParams() const{

}

TConfigParam TnewaeDevice::setPostInitParams(TConfigParam params){

}

size_t TnewaeDevice::writeData(const uint8_t * buffer, size_t len){

}

size_t TnewaeDevice::readData(uint8_t * buffer, size_t len){

}
