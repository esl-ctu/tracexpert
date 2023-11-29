#include "tfiledevice.h"
#include "tfile.h"

TFileDevice::TFileDevice(QString & name, QString & info, TFile & tFile):
    m_createdManually(true), m_openMode(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text), m_file(), m_fileInfo(), m_name(name), m_info(info), m_initialized(false), m_tFile(tFile)
{

    // Pre-init parameter File path is editable and pre-filled with passed "name" variable
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("File path", m_name, TConfigParam::TType::TString, "File path (e.g. C:/Users/novak/Documents/data.csv)", false));

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
    if(m_initialized) {
        this->deInit();
    }
}

QString TFileDevice::getName() const {
    return m_name;
}

QString TFileDevice::getInfo() const {
    return m_info;
}


TConfigParam TFileDevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TFileDevice::setPreInitParams(TConfigParam params) {

    if(m_initialized){
        params.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
        return params;
    }

    if(!_validatePreInitParamsStructure(params)){
        params.setState(TConfigParam::TState::TError, "Wrong structure of the pre-init params for FileDevice");
        return params;
    }

    m_preInitParams = params;
    m_preInitParams.resetState();

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
        m_preInitParams.getSubParamByName("Read/Write mode")->setState(TConfigParam::TState::TError, "Invalid enum value for Read/Write mode.");
    }

    // Set Write behaviour
    QString wbehavParam = m_preInitParams.getSubParamByName("Write behaviour")->getValue();

    m_preInitParams.getSubParamByName("Write behaviour")->resetState();
    if(wbehavParam == "Append") {
        newOpenMode |= QIODevice::Append;
    } else if(wbehavParam == "Truncate") {
        newOpenMode |= QIODevice::Truncate;
    } else {
        m_preInitParams.getSubParamByName("Write behaviour")->setState(TConfigParam::TState::TError, "Invalid enum value for Write behaviour.");
    }

    // Set Type of file
    QString ftypeParam = m_preInitParams.getSubParamByName("Type of file")->getValue();

    m_preInitParams.getSubParamByName("Type of file")->resetState();
    if(ftypeParam == "Text") {
        newOpenMode |= QIODevice::Text;
    } else if(ftypeParam == "Binary") {
        // binary mode is default behaviour, no need to set any OpenMode flag
    } else {
        m_preInitParams.getSubParamByName("Type of file")->setState(TConfigParam::TState::TError, "Invalid enum value for Type of file.");
    }

    m_openMode = newOpenMode;

    return m_preInitParams;
}

void TFileDevice::init(bool *ok) {

    if(m_initialized) {
        qWarning("TFileDevice was already initialized when init was called!");

        if(ok != nullptr) *ok = false;
        return;
    }

    bool iok = false;

    _openFile(&iok);

    if(!iok) {
        qWarning("Failed to open the file");
    }

    if(iok) {
        _createPostInitParams();

        m_initialized = true;

        if(ok != nullptr) *ok = true;      

    } else {
        if(ok != nullptr) *ok = false;
    }    

}

void TFileDevice::_openFile(bool *ok) {

    bool iok = false;

    if(m_createdManually){

        TConfigParam * locationParam = m_preInitParams.getSubParamByName("File path", &iok);
        if(!iok) {
            qWarning("File path parameter not found in the pre-init config.");
            if(ok != nullptr) *ok = false;
            return;
        }

        m_file.setFileName(locationParam->getValue());

    } else {
        m_file.setFileName(m_fileInfo.fileName());
    }

    if(m_openMode.testFlag(QIODevice::ReadWrite) || m_openMode.testFlag(QIODevice::WriteOnly)) {
        if(!m_tFile.registerOpenFile(m_file.filesystemFileName())) {
            qWarning("Failed to open file; is it already open for writing?");
            if(ok != nullptr) *ok = false;
            return;
        }
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

    if(!m_initialized) {
        qWarning("TFileDevice was not initialized when deInit was called!");

        if(ok != nullptr) *ok = false;
        return;
    }

    m_tFile.unregisterOpenFile(m_file.filesystemFileName());

    m_file.close();

    m_initialized = false;

    if(ok != nullptr) *ok = true;
}

TConfigParam TFileDevice::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TFileDevice::setPostInitParams(TConfigParam params) {

    if(!m_initialized) {
        params.setState(TConfigParam::TState::TError, "Cannot set post-init parameters on an uninitialized device.");
        return params;
    }

    if(!_validatePostInitParamsStructure(params)) {
        params.setState(TConfigParam::TState::TError, "Wrong structure of the post-init params for FileDevice");
        return params;
    }

    m_postInitParams = params;

    qint64 posToSeek = params.getSubParamByName("Seek to position")->getValue().toInt();
    if(posToSeek > -1) {
        if(!m_file.seek(posToSeek)) {
            m_postInitParams.getSubParamByName("Seek to position")->setState(TConfigParam::TState::TError, "Failed to seek to supplied position.");
        }

        m_postInitParams.getSubParamByName("Seek to position")->setValue(m_file.pos());
    }

    return m_postInitParams;

}

size_t TFileDevice::writeData(const uint8_t * buffer, size_t len){

    qsizetype writtenLen;
    writtenLen = m_file.write((const char *) buffer, len);

    if(writtenLen != len) {
        qWarning("Failed to write as much data as requested to the file.");
    }

    m_postInitParams.getSubParamByName("Seek to position")->setValue(m_file.pos());

    return writtenLen < 0 ? 0 : writtenLen;

}

size_t TFileDevice::readData(uint8_t * buffer, size_t len) {

    qsizetype readLen;
    readLen = m_file.read((char *) buffer, len);

    if(readLen != len) {
        qWarning("Failed to read as much data as requested from the file.");
    }

    m_postInitParams.getSubParamByName("Seek to position")->setValue(m_file.pos());

    return readLen < 0 ? 0 : readLen;

}
