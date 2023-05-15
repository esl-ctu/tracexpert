#include "tfiledevice.h"

TFileDevice::TFileDevice(QString & name, QString & info):
    m_createdManually(true), m_openMode(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text), m_file(), m_fileInfo(), m_name(name), m_info(info), m_initialized(false)
{
    // Pre-init parameters with system location (pre-filled with port name)
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("File path", m_name, TConfigParam::TType::TString, "File path (e.g. C:\\Users\\novak\\Documents\\data.csv)", false));

    _createPreInitParams();
}

// Probably does not make sense to have this constructor as files will not be auto-detected
TFileDevice::TFileDevice(const QFileInfo &fileInfo):
    m_createdManually(false), m_openMode(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text), m_file(), m_fileInfo(fileInfo), m_name(fileInfo.filePath()),
    m_info(fileInfo.filePath() + ", file is " + (fileInfo.isReadable() ? "readable" : "NOT readable") + " and " + (fileInfo.isWritable() ? "writable" : "NOT writable")), m_initialized(false)
{

    // Pre-init parameters are all read-only for automatically detected devices
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("File path", m_name, TConfigParam::TType::TString, "File path (e.g. C:\\Users\\novak\\Documents\\data.csv)", true));

    _createPreInitParams();
}

void TFileDevice::_createPreInitParams(){

    TConfigParam rwmodeParam = TConfigParam("Read/Write mode", "ReadWrite", TConfigParam::TType::TEnum, "In which R/W mode to open the file (options are ReadOnly, WriteOnly, ReadWrite)", false);
    rwmodeParam.addEnumValue("ReadOnly");
    rwmodeParam.addEnumValue("WriteOnly");
    rwmodeParam.addEnumValue("ReadWrite");
    m_preInitParams.addSubParam(rwmodeParam);

    TConfigParam wbehavParam = TConfigParam("Write behaviour", "Append", TConfigParam::TType::TEnum, "How write operations should behave (options are Append, Truncate)", false);
    wbehavParam.addEnumValue("Append");
    wbehavParam.addEnumValue("Truncate");
    m_preInitParams.addSubParam(wbehavParam);

    TConfigParam ftypeParam = TConfigParam("Type of file", "Text", TConfigParam::TType::TEnum, "How the file should be treated (options are Text, Binary)", false);
    ftypeParam.addEnumValue("Text");
    ftypeParam.addEnumValue("Binary");
    m_preInitParams.addSubParam(ftypeParam);

}

bool TFileDevice::_validatePreInitParamsStructure(TConfigParam & params) {

    // Only checks the structure of parameters. Values are validated later during init. Enum values are checked during their setting by the TConfigParam.

    bool iok = false;

    params.getSubParamByName("File path", &iok);
    if(!iok) return false;

    params.getSubParamByName("Read/Write mode", &iok);
    if(!iok) return false;

    params.getSubParamByName("Write behaviour", &iok);
    if(!iok) return false;

    params.getSubParamByName("Type of file", &iok);
    if(!iok) return false;

    return true;

}

TFileDevice::~TFileDevice() {
    // QFile destructor closes the file, if necessary, and then destroys object.
    // Nothing to do.
}

QString TFileDevice::getIODeviceName() const {
    return m_name;
}

QString TFileDevice::getIODeviceInfo() const {
    return m_info;
}


TConfigParam TFileDevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TFileDevice::setPreInitParams(TConfigParam params) {

    if(m_initialized){
        QString error = "Cannot change pre-init parameters on an initialized device.";
        params.setState(TConfigParam::TState::TError, error);
        qCritical(error.toLatin1());
        return params;
    }

    if(!_validatePreInitParamsStructure(params)){
        QString error = "Wrong structure of the pre-init params for FileDevice";
        qCritical(error.toLatin1());
        params.setState(TConfigParam::TState::TError, error);
        return params;
    }

    m_preInitParams = params;
    m_preInitParams.resetState();

    bool iok = true;

    QFlags<QIODevice::OpenModeFlag> newOpenMode;

    // Set R/W mode
    QString rwmodeParam = m_preInitParams.getSubParamByName("Read/Write mode")->getValue();

    m_preInitParams.getSubParamByName("Read/Write mode")->resetState();
    if(rwmodeParam == "ReadOnly") {
        newOpenMode |= QIODevice::ReadOnly;
    } else if(rwmodeParam == "WriteOnly") {
        newOpenMode |= QIODevice::WriteOnly;
    } else if(rwmodeParam == "ReadWrite") {
        newOpenMode |= QIODevice::ReadWrite;
    } else {
        QString error = "Invalid enum value for Read/Write mode.";
        qCritical(error.toLatin1());
        m_preInitParams.getSubParamByName("Read/Write mode")->setState(TConfigParam::TState::TError, error);
        iok = false;
    }

    // Set Write behaviour
    QString wbehavParam = m_preInitParams.getSubParamByName("Write behaviour")->getValue();

    m_preInitParams.getSubParamByName("Write behaviour")->resetState();
    if(wbehavParam == "Append") {
        newOpenMode |= QIODevice::Append;
    } else if(wbehavParam == "Truncate") {
        newOpenMode |= QIODevice::Truncate;
    } else {
        QString error = "Invalid enum value for Write behaviour.";
        qCritical(error.toLatin1());
        m_preInitParams.getSubParamByName("Write behaviour")->setState(TConfigParam::TState::TError, error);
        iok = false;
    }

    // Set Type of file
    QString ftypeParam = m_preInitParams.getSubParamByName("Type of file")->getValue();

    m_preInitParams.getSubParamByName("Type of file")->resetState();
    if(ftypeParam == "Text") {
        newOpenMode |= QIODevice::Text;
    } else if(ftypeParam == "Binary") {
        // binary mode is default behaviour, no need to set any OpenMode flag
    } else {
        QString error = "Invalid enum value for Type of file.";
        qCritical(error.toLatin1());
        m_preInitParams.getSubParamByName("Type of file")->setState(TConfigParam::TState::TError, error);
        iok = false;
    }

    if(iok) {
        m_openMode = newOpenMode;
    } else {
        QString error = "One or more pre-init parameters have invalid values.";
        qCritical(error.toLatin1());
        m_preInitParams.setState(TConfigParam::TState::TError, error);
    }

    return m_preInitParams;
}

void TFileDevice::init(bool *ok) {

    bool iok = false;

    _openFile(&iok);

    if(!iok) {
        qWarning("Failed to open the file");
    }

    if(iok){
        _createPostInitParams();
        if(ok != nullptr) *ok = true;
    } else {
        if(ok != nullptr) *ok = false;
    }

    m_initialized = true;

}

void TFileDevice::_openFile(bool *ok) {

    bool iok = false;

    // Select serial port
    if(m_createdManually){

        TConfigParam * locationParam = m_preInitParams.getSubParamByName("File path", &iok);
        if(!iok){
            qWarning("File path parameter not found in the pre-init config.");
            if(ok != nullptr) *ok = false;
            return;
        }

        m_file.setFileName(locationParam->getValue());

    } else {
        m_file.setFileName(m_fileInfo.fileName());
    }

    // Open file
    iok = m_file.open(m_openMode);

    if(!iok) {
        qWarning((QString("Failed to open file: ") + m_file.errorString()).toLatin1());
        if(ok != nullptr) *ok = false;
        return;
    }

    if(ok != nullptr) *ok = true;

}

void TFileDevice::_createPostInitParams(){

    m_postInitParams = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");

    m_postInitParams.addSubParam(TConfigParam("Seek to position", "-1", TConfigParam::TType::TInt, "Position to seek to (-1 for no seek)."));
}

bool TFileDevice::_validatePostInitParamsStructure(TConfigParam & params) {

    // Only checks the structure of parameters. Values are validated later during init. Enum values are checked during their setting by the TConfigParam.

    bool iok = false;

    params.getSubParamByName("Seek to position", &iok);
    if(!iok) return false;

    return true;

}


void TFileDevice::deInit(bool *ok) {

    m_file.close();
    m_initialized = true;

}

TConfigParam TFileDevice::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TFileDevice::setPostInitParams(TConfigParam params) {

    if(!m_initialized){
        QString error = "Cannot set post-init parameters on an uninitialized device.";
        params.setState(TConfigParam::TState::TError, error);
        qCritical(error.toLatin1());
        return params;
    }

    if(!_validatePostInitParamsStructure(params)){
        qCritical("Wrong structure of the post-init params for FileDevice");
        return params;
    }

    m_postInitParams = params;

    qint64 posToSeek = params.getSubParamByName("Seek to position")->getValue().toInt();
    if(posToSeek > -1) m_file.seek(posToSeek);

    return m_postInitParams;

}

size_t TFileDevice::writeData(const uint8_t * buffer, size_t len){

    size_t writtenToBuffer;
    writtenToBuffer = m_file.write((const char *) buffer, len);
    return writtenToBuffer;

}

size_t TFileDevice::readData(uint8_t * buffer, size_t len) {

    size_t readLen;
    readLen = m_file.read((char *) buffer, len);
    return readLen;

}
