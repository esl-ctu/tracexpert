#include "tserialportdevice.h"

TSerialPortDevice::TSerialPortDevice(QString & name, QString & info):
    m_createdManually(true), m_port(), m_portInfo(), m_name(name), m_info(info), m_readTimeout(5000), m_writeTimeout(5000), m_initialized(false)
{

    // Pre-init parameters with system location (pre-filled with port name)
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("System location", m_name, TConfigParam::TType::TString, "System location of the serial port (e.g. COM1, \\\\.\\COM12 or /dev/ttyUSB0", false));

}

TSerialPortDevice::TSerialPortDevice(const QSerialPortInfo &portInfo):
    m_createdManually(false), m_port(), m_portInfo(portInfo), m_name(portInfo.portName()), m_info(portInfo.description() + " " + portInfo.manufacturer() + " " + portInfo.serialNumber()), m_readTimeout(5000), m_writeTimeout(5000), m_initialized(false)
{

    // Pre-init parameters are all read-only for automatically detected devices
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("System location", portInfo.systemLocation(), TConfigParam::TType::TString, "System location of the serial port (e.g. \\\\.\\COM12 or /dev/ttyUSB0", true));
    m_preInitParams.addSubParam(TConfigParam("Description", portInfo.description(), TConfigParam::TType::TString, "Serial port description", true));
    m_preInitParams.addSubParam(TConfigParam("Manufacturer", portInfo.manufacturer(), TConfigParam::TType::TString, "Serial port manufacturer", true));
    m_preInitParams.addSubParam(TConfigParam("Serial Number", portInfo.serialNumber(), TConfigParam::TType::TString, "Serial port serial number", true));
    m_preInitParams.addSubParam(TConfigParam("Vendor Identifier", (portInfo.hasVendorIdentifier() ? QByteArray::number(portInfo.vendorIdentifier(), 16) : QByteArray()), TConfigParam::TType::TString, "Serial port vendor identifier", true));
    m_preInitParams.addSubParam(TConfigParam("Product Identifier", (portInfo.hasProductIdentifier() ? QByteArray::number(portInfo.productIdentifier(), 16) : QByteArray()), TConfigParam::TType::TString, "Serial port product identifier", true));

}

TSerialPortDevice::~TSerialPortDevice() {
    // QSerialPort destructor closes the serial port, if necessary, and then destroys object.
    // Nothing to do.
}

QString TSerialPortDevice::getIODeviceName() const {
    return m_name;
}

QString TSerialPortDevice::getIODeviceInfo() const {
    return m_info;
}


TConfigParam TSerialPortDevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TSerialPortDevice::setPreInitParams(TConfigParam params) {

    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

void TSerialPortDevice::init(bool *ok) {

    bool iok = false;

    _openPort(&iok);

    if(!iok) {
        qWarning("Failed to open the serial port");
    }

    if(iok){
        _createPostInitParams();
        if(ok != nullptr) *ok = true;
    } else {
        if(ok != nullptr) *ok = false;
    }

    m_initialized = true;

}

void TSerialPortDevice::_openPort(bool *ok) {

    bool iok = false;

    // Select serial port
    if(m_createdManually){

        TConfigParam * locationParam = m_preInitParams.getSubParamByName("System location", &iok);
        if(!iok){
            qWarning("System location parameter not found in the pre-init config.");
            if(ok != nullptr) *ok = false;
            return;
        }

        m_port.setPortName(locationParam->getValue());

    } else {

        m_port.setPort(m_portInfo);

    }

    // Open serial port
    iok = m_port.open(QIODeviceBase::ReadWrite);

    if(!iok) {
        qWarning((QString("Failed to open the specified serial port: ") + m_port.errorString()).toLatin1());
        if(ok != nullptr) *ok = false;
        return;
    }

    if(ok != nullptr) *ok = true;

}

void TSerialPortDevice::_createPostInitParams(){

    m_postInitParams = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");

    TConfigParam baudrateParam = TConfigParam("Baudrate", "9600", TConfigParam::TType::TEnum, "Baudate of the serial port.");
    baudrateParam.addEnumValue("1200");
    baudrateParam.addEnumValue("2400");
    baudrateParam.addEnumValue("4800");
    baudrateParam.addEnumValue("9600");
    baudrateParam.addEnumValue("19200");
    baudrateParam.addEnumValue("38400");
    baudrateParam.addEnumValue("57600");
    baudrateParam.addEnumValue("115200");
    baudrateParam.addEnumValue("Custom");
    m_postInitParams.addSubParam(baudrateParam);

    m_postInitParams.addSubParam(TConfigParam("Custom baudrate", "0", TConfigParam::TType::TUInt, "The custom baudrate must be selected in the Baudrate field. The value must be supported by the hardware."));

    TConfigParam parityParam = TConfigParam("Parity", "None", TConfigParam::TType::TEnum, "Parity");
    parityParam.addEnumValue("None");
    parityParam.addEnumValue("Even");
    parityParam.addEnumValue("Odd");
    parityParam.addEnumValue("Space");
    parityParam.addEnumValue("Mark");
    m_postInitParams.addSubParam(parityParam);

    TConfigParam stopParam = TConfigParam("Stop bits", "One", TConfigParam::TType::TEnum, "Stop bits");
    stopParam.addEnumValue("One");
    stopParam.addEnumValue("One and half");
    stopParam.addEnumValue("Two");
    m_postInitParams.addSubParam(stopParam);

    TConfigParam flowParam = TConfigParam("Flow control", "None", TConfigParam::TType::TEnum, "Flow control");
    flowParam.addEnumValue("None");
    flowParam.addEnumValue("Hardware (RTS/CTS)");
    flowParam.addEnumValue("Software (XON/XOFF)");
    m_postInitParams.addSubParam(flowParam);

    m_postInitParams.addSubParam(TConfigParam("Data bits", "8", TConfigParam::TType::TString, "Data bits", true));

    m_postInitParams.addSubParam(TConfigParam("Read timeout (ms)", "5000", TConfigParam::TType::TInt, "Timeout while waiting for data to be read from the serial port (-1 for no timeout)."));
    m_postInitParams.addSubParam(TConfigParam("Write timeout (ms)", "5000", TConfigParam::TType::TInt, "Timeout while waiting for data to be written to the serial port (-1 for no timeout)."));

}

bool TSerialPortDevice::_validatePostInitParamsStructure(TConfigParam & params) {

    // Only checks the structure of parameters. Values are validated later during init. Enum values are checked during their setting by the TConfigParam.

    bool iok = false;

    params.getSubParamByName("Baudrate", &iok);
    if(!iok) return false;

    params.getSubParamByName("Custom baudrate", &iok);
    if(!iok) return false;

    params.getSubParamByName("Parity", &iok);
    if(!iok) return false;

    params.getSubParamByName("Stop bits", &iok);
    if(!iok) return false;

    params.getSubParamByName("Flow control", &iok);
    if(!iok) return false;

    params.getSubParamByName("Data bits", &iok);
    if(!iok) return false;

    params.getSubParamByName("Read timeout (ms)", &iok);
    if(!iok) return false;

    params.getSubParamByName("Write timeout (ms)", &iok);
    if(!iok) return false;

    return true;

}


void TSerialPortDevice::deInit(bool *ok) {

    m_port.close();
    m_initialized = true;

}

TConfigParam TSerialPortDevice::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TSerialPortDevice::setPostInitParams(TConfigParam params) {

    if(!_validatePostInitParamsStructure(params)){
        qCritical("Wrong structure of the post-init params for SerialPortDevice");
        return m_postInitParams;
    }

    m_postInitParams = params;

    bool iok = false, iok2 = false;

    // Baudrate

    QString baudrateParam = m_postInitParams.getSubParamByName("Baudrate")->getValue(); // getSubParamByName != nullptr, checked by _validatePostInitParamsStructure; otherwise, need a check!
    QString customBaudrateParam = m_postInitParams.getSubParamByName("Custom baudrate")->getValue();

    // Set the baudrate
    if(baudrateParam == "Custom") {
        iok2 = m_port.setBaudRate(customBaudrateParam.toInt(&iok));
    } else {
        iok2 = m_port.setBaudRate(baudrateParam.toInt(&iok));
    }

    // Check the baudrate, set/reset param warnings
    if(baudrateParam == "Custom") {

        if(!iok || !iok2 || m_port.baudRate() != customBaudrateParam.toInt()){
            QString error = "Failed to set the required custom baudrate: " + m_port.errorString();
            qWarning(error.toLatin1());
            m_postInitParams.getSubParamByName("Baudrate")->resetState();
            m_postInitParams.getSubParamByName("Custom baudrate")->setState(TConfigParam::TState::TWarning, error);
        } else {
            m_postInitParams.getSubParamByName("Baudrate")->resetState();
            m_postInitParams.getSubParamByName("Custom baudrate")->resetState();
        }

        m_postInitParams.getSubParamByName("Custom baudrate")->setValue(QString::number(m_port.baudRate()));

    } else {

        if(!iok || !iok2 || m_port.baudRate() != baudrateParam.toInt()){
            QString error = "Failed to set the required baudrate: " + m_port.errorString();
            qWarning(error.toLatin1());
            m_postInitParams.getSubParamByName("Baudrate")->setState(TConfigParam::TState::TWarning, error);
            m_postInitParams.getSubParamByName("Custom baudrate")->resetState();
        } else {
            m_postInitParams.getSubParamByName("Baudrate")->resetState();
            m_postInitParams.getSubParamByName("Custom baudrate")->resetState();
        }

        if(customBaudrateParam != "0"){
            qWarning("The custom baudrate is set but ignored, because Baudrate is not set to Custom.");
            m_postInitParams.getSubParamByName("Custom baudrate")->setState(TConfigParam::TState::TWarning, "This value is ignored, because the Baudrate param is not set to Custom.");
        }

        m_postInitParams.getSubParamByName("Baudrate")->setValue(QString::number(m_port.baudRate()));

    }

    // Parity

    QString parityParam = m_postInitParams.getSubParamByName("Parity")->getValue();

    // Set parity
    if(parityParam == "None"){
        iok = m_port.setParity(QSerialPort::NoParity);
    } else if(parityParam == "Even"){
        iok = m_port.setParity(QSerialPort::EvenParity);
    } else if(parityParam == "Odd"){
        iok = m_port.setParity(QSerialPort::OddParity);
    } else if(parityParam == "Space"){
        iok = m_port.setParity(QSerialPort::SpaceParity);
    } else if(parityParam == "Mark"){
        iok = m_port.setParity(QSerialPort::MarkParity);
    } else {
        iok = false;
    }

    // Check the parity, set/reset warnings
    QSerialPort::Parity actualParity = m_port.parity();
    QString actualParityString = "";
    switch(actualParity){
        case QSerialPort::NoParity:
            actualParityString = "None";
            break;
        case QSerialPort::EvenParity:
            actualParityString = "Even";
            break;
        case QSerialPort::OddParity:
            actualParityString = "Odd";
            break;
        case QSerialPort::SpaceParity:
            actualParityString = "Space";
            break;
        case QSerialPort::MarkParity:
            actualParityString = "Mark";
            break;
        default:
            qCritical("Unexpected actual parity settings");
            break;
    }
    if(!iok || actualParityString != parityParam){
        QString error = "Failed to set the required parity: " + m_port.errorString();
        qWarning(error.toLatin1());
        m_postInitParams.getSubParamByName("Parity")->setState(TConfigParam::TState::TWarning, error);
    } else {
        m_postInitParams.getSubParamByName("Parity")->resetState();
    }
    m_postInitParams.getSubParamByName("Parity")->setValue(actualParityString);

    // Stop bits

    QString stopbitsParam = m_postInitParams.getSubParamByName("Stop bits")->getValue();

    // Set stop bits
    if(stopbitsParam == "One") {
        iok = m_port.setStopBits(QSerialPort::OneStop);
    } else if(stopbitsParam == "One and half") {
        iok = m_port.setStopBits(QSerialPort::OneAndHalfStop);
    } else if(stopbitsParam == "Two") {
        iok = m_port.setStopBits(QSerialPort::TwoStop);
    } else {
        iok = false;
    }

    // Check the stop bits, set/reset warnings
    QSerialPort::StopBits actualStopbits = m_port.stopBits();
    QString actualStopbitsString = "";
    switch(actualStopbits){
        case QSerialPort::OneStop:
            actualStopbitsString = "One";
            break;
        case QSerialPort::OneAndHalfStop:
            actualStopbitsString = "One and half";
            break;
        case QSerialPort::TwoStop:
            actualStopbitsString = "Two";
            break;
        default:
            qCritical("Unexpected stop bits settings");
            break;
    }
    if(!iok || actualStopbitsString != stopbitsParam){
        QString error = "Failed to set the required stop bits: " + m_port.errorString();
        qWarning(error.toLatin1());
        m_postInitParams.getSubParamByName("Stop bits")->setState(TConfigParam::TState::TWarning, error);
    } else {
        m_postInitParams.getSubParamByName("Stop bits")->resetState();
    }
    m_postInitParams.getSubParamByName("Stop bits")->setValue(actualStopbitsString);

    // Flow control

    QString flowcontrolParam = m_postInitParams.getSubParamByName("Flow control")->getValue();

    // Set flow control
    if(flowcontrolParam == "None"){
        iok = m_port.setFlowControl(QSerialPort::NoFlowControl);
    } else if(flowcontrolParam == "Hardware (RTS/CTS)"){
        iok = m_port.setFlowControl(QSerialPort::HardwareControl);
    } else if(flowcontrolParam == "Software (XON/XOFF)"){
        iok = m_port.setFlowControl(QSerialPort::SoftwareControl);
    } else {
        iok = false;
    }

    // Check the flow control, set/reset warnings
    QSerialPort::FlowControl actualFlowcontrol = m_port.flowControl();
    QString actualFlowcontrolString = "";
    switch(actualFlowcontrol) {
        case QSerialPort::NoFlowControl:
            actualFlowcontrolString = "None";
            break;
        case QSerialPort::HardwareControl:
            actualFlowcontrolString = "Hardware (RTS/CTS)";
            break;
        case QSerialPort::SoftwareControl:
            actualFlowcontrolString = "Software (XON/XOFF)";
            break;
        default:
            qCritical("Unexpected flow control settings");
            break;
    }
    if(!iok || actualFlowcontrolString != flowcontrolParam){
        QString error = "Failed to set the required flow control: " + m_port.errorString();
        qWarning(error.toLatin1());
        m_postInitParams.getSubParamByName("Flow control")->setState(TConfigParam::TState::TWarning, error);
    } else {
        m_postInitParams.getSubParamByName("Flow control")->resetState();
    }
    m_postInitParams.getSubParamByName("Flow control")->setValue(actualFlowcontrolString);

    // Data bits

    // Read-only parameter, only 8 data bits are supported
    if(m_port.dataBits() != QSerialPort::Data8){
        qCritical("Unexpected data bits settings");
    }

    // Timeout
    m_readTimeout = params.getSubParamByName("Read timeout (ms)")->getValue().toUInt();
    if(m_readTimeout < 0) m_readTimeout = -1;
    if(m_readTimeout == 0) m_readTimeout = 1;

    m_writeTimeout = params.getSubParamByName("Write timeout (ms)")->getValue().toUInt();
    if(m_writeTimeout < 0) m_writeTimeout = -1;
    if(m_writeTimeout == 0) m_writeTimeout = 1;

    return m_postInitParams;

}

size_t TSerialPortDevice::writeData(const uint8_t * buffer, size_t len){

    size_t writtenToBuffer;
    bool iok;

    writtenToBuffer = m_port.write((const char *) buffer, len);

    if(writtenToBuffer != len){
        qWarning("Failed to write all the data to the serial port buffer.");
    }

    QElapsedTimer stopwatch;
    stopwatch.start();

    iok = m_port.waitForBytesWritten(m_writeTimeout);

    while((m_port.bytesToWrite() != 0) && ((m_writeTimeout > 0) ? (stopwatch.elapsed() < m_writeTimeout) : true)) {

        iok &= m_port.waitForBytesWritten((m_writeTimeout > 0) ? (m_writeTimeout - stopwatch.elapsed()) : m_writeTimeout);

    }

    if(!iok || m_port.bytesToWrite() != 0) {
        qWarning("Failed to send the data from the serial port buffer (possibly timeout).");
    }

    return writtenToBuffer;
}

size_t TSerialPortDevice::readData(uint8_t * buffer, size_t len) {

    size_t readLen;

    QElapsedTimer stopwatch;
    stopwatch.start();

    if(m_port.waitForReadyRead(m_readTimeout)){

        readLen = m_port.read((char *) buffer, len);      

        while((readLen < len) && ((m_readTimeout > 0) ? (stopwatch.elapsed() < m_readTimeout) : true) && (m_port.waitForReadyRead((m_readTimeout > 0) ? (m_readTimeout - stopwatch.elapsed()) : m_readTimeout))) {
            readLen += m_port.read((char *) buffer + readLen, len - readLen);

        }

        if(len != readLen){
            //qWarning("Failed to read as much data as requested from the serial port (timeout).");
        }
        if(m_port.bytesAvailable() > 0){
            qWarning("Unread data left in the serial port buffer after reading.");
        }

        return readLen;

    } else {
        //qWarning("Failed to read any data from the serial port (timeout).");
        return 0;
    }

}
