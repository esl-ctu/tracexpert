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
// Petr Socha (initial author)
// David Pokorný

#include "tserialportdevice.h"

TSerialPortDevice::TSerialPortDevice(QString & name, QString & info):
    m_createdManually(true), m_portInfo(), m_name(name), m_info(info), m_readTimeout(5000), m_writeTimeout(5000), m_initialized(false), m_osHandle(0)
{

    // Pre-init parameters with system location (pre-filled with port name)
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("System location", m_name, TConfigParam::TType::TString, "System location of the serial port (e.g. COM1, \\\\.\\COM12 or /dev/ttyUSB0", false));

}

TSerialPortDevice::TSerialPortDevice(const QSerialPortInfo &portInfo):
    m_createdManually(false), m_portInfo(portInfo), m_name(portInfo.portName()), m_info(portInfo.description() + " " + portInfo.manufacturer() + " " + portInfo.serialNumber()), m_readTimeout(5000), m_writeTimeout(5000), m_initialized(false), m_osHandle(0)
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
    (*this).deInit();
}

QString TSerialPortDevice::getName() const {
    return m_name;
}

QString TSerialPortDevice::getInfo() const {
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
        m_initialized = true;
        _createPostInitParams();
        setPostInitParams(m_postInitParams); // set default post-init params
        if(ok != nullptr) *ok = true;
    } else {
        if(ok != nullptr) *ok = false;
    }

}

void TSerialPortDevice::_openPort(bool *ok) {

    bool iok = false;

    TConfigParam * locationParam = m_preInitParams.getSubParamByName("System location", &iok);
    if(!iok){
        qCritical("System location parameter not found in the pre-init config.");
        if(ok != nullptr) *ok = false;
        return;
    }

    // Opening the port
#ifdef _WIN32

    m_osHandle = CreateFileA(qPrintable(locationParam->getValue()), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if (m_osHandle == INVALID_HANDLE_VALUE) {
        //CloseHandle(m_osHandle);
        qCritical("Failed to open the specified serial port");
        if(ok != nullptr) *ok = false;
        return;
    }

#else

    m_osHandle = open(qPrintable(locationParam->getValue()), O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

    if(m_osHandle < 0){
        qCritical("Failed to open the specified serial port");
        if(ok != nullptr) *ok = false;
        return;
    }
    if(!isatty(m_osHandle)){
        close(m_osHandle);
        qCritical("Specified filename is not a serial port");
        if(ok != nullptr) *ok = false;
        return;
    }

    if(fcntl(m_osHandle, F_SETFL, 0) < 0){
        close(m_osHandle);
        qCritical("Could not reset the serial port flags");
        if(ok != nullptr) *ok = false;
        return;
    }

#endif

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
#ifdef _WIN32
    baudrateParam.addEnumValue("128000");
    baudrateParam.addEnumValue("256000");
    baudrateParam.addEnumValue("Custom");
#endif

    m_postInitParams.addSubParam(baudrateParam);

#ifdef _WIN32
    m_postInitParams.addSubParam(TConfigParam("Custom baudrate", "0", TConfigParam::TType::TUInt, "The custom baudrate must be selected in the Baudrate field. The value must be supported by the hardware."));
#endif

    TConfigParam parityParam = TConfigParam("Parity", "None", TConfigParam::TType::TEnum, "Parity");
    parityParam.addEnumValue("None");
    parityParam.addEnumValue("Even");
    parityParam.addEnumValue("Odd");
    //parityParam.addEnumValue("Space");
    //parityParam.addEnumValue("Mark");
    m_postInitParams.addSubParam(parityParam);

    TConfigParam stopParam = TConfigParam("Stop bits", "One", TConfigParam::TType::TEnum, "Stop bits");
    stopParam.addEnumValue("One");
    //stopParam.addEnumValue("One and half");
    stopParam.addEnumValue("Two");
    m_postInitParams.addSubParam(stopParam);

    /*TConfigParam flowParam = TConfigParam("Flow control", "None", TConfigParam::TType::TEnum, "Flow control");
    flowParam.addEnumValue("None");
    flowParam.addEnumValue("Hardware (RTS/CTS)");
    flowParam.addEnumValue("Software (XON/XOFF)");
    m_postInitParams.addSubParam(flowParam);*/

    m_postInitParams.addSubParam(TConfigParam("Data bits", "8", TConfigParam::TType::TString, "Data bits", true));

#ifdef _WIN32
    m_postInitParams.addSubParam(TConfigParam("Read timeout (ms)", "500", TConfigParam::TType::TInt, "Timeout while waiting for data to be read from the serial port."));
    m_postInitParams.addSubParam(TConfigParam("Write timeout (ms)", "500", TConfigParam::TType::TInt, "Timeout while waiting for data to be written to the serial port."));
#else
    m_postInitParams.addSubParam(TConfigParam("Timeout (ms)", "500", TConfigParam::TType::TInt, "Timeout while waiting for data to be read from the serial port."));
#endif

}

bool TSerialPortDevice::_validatePostInitParamsStructure(TConfigParam & params) {

    // Only checks the structure of parameters. Values are validated later during init. Enum values are checked during their setting by the TConfigParam.

    bool iok = false;

    params.getSubParamByName("Baudrate", &iok);
    if(!iok) return false;
#ifdef _WIN32
    params.getSubParamByName("Custom baudrate", &iok);
    if(!iok) return false;
#endif
    params.getSubParamByName("Parity", &iok);
    if(!iok) return false;

    params.getSubParamByName("Stop bits", &iok);
    if(!iok) return false;

    /*params.getSubParamByName("Flow control", &iok);
    if(!iok) return false;*/

    params.getSubParamByName("Data bits", &iok);
    if(!iok) return false;

#ifdef _WIN32
    params.getSubParamByName("Read timeout (ms)", &iok);
    if(!iok) return false;

    params.getSubParamByName("Write timeout (ms)", &iok);
    if(!iok) return false;
#else
    params.getSubParamByName("Timeout (ms)", &iok);
    if(!iok) return false;
#endif

    return true;

}


void TSerialPortDevice::deInit(bool *ok) {

    if(m_initialized == true){

#ifdef _WIN32

    CloseHandle(m_osHandle);
    m_osHandle = nullptr;

#else

    tcflush(m_osHandle, TCIOFLUSH);
    close(m_osHandle);

#endif

    }

    m_initialized = false;
    if(ok != nullptr) *ok = true;

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

    m_postInitParams.resetState(true);

    // Params values
    QString baudrateParam = m_postInitParams.getSubParamByName("Baudrate")->getValue(); // getSubParamByName != nullptr, checked by _validatePostInitParamsStructure; otherwise, need a check!
#ifdef _WIN32
    QString customBaudrateParam = m_postInitParams.getSubParamByName("Custom baudrate")->getValue();
#endif
    QString parityParam = m_postInitParams.getSubParamByName("Parity")->getValue();
    QString stopbitsParam = m_postInitParams.getSubParamByName("Stop bits")->getValue();

 #ifdef _WIN32
    m_readTimeout = params.getSubParamByName("Read timeout (ms)")->getValue().toUInt();
    if(m_readTimeout < 0) m_readTimeout = -1;
    if(m_readTimeout == 0) m_readTimeout = 1;

    m_writeTimeout = params.getSubParamByName("Write timeout (ms)")->getValue().toUInt();
    if(m_writeTimeout < 0) m_writeTimeout = -1;
    if(m_writeTimeout == 0) m_writeTimeout = 1;
#else
    m_readTimeout = params.getSubParamByName("Timeout (ms)")->getValue().toUInt();
    if(m_readTimeout < 0) m_readTimeout = -1;
    if(m_readTimeout == 0) m_readTimeout = 1;

    m_writeTimeout = params.getSubParamByName("Timeout (ms)")->getValue().toUInt();
    if(m_writeTimeout < 0) m_writeTimeout = -1;
    if(m_writeTimeout == 0) m_writeTimeout = 1;
#endif


#ifdef _WIN32

    BOOL status;

    DCB serialParams = { 0 };
    serialParams.DCBlength = sizeof(serialParams);

    status = GetCommState(m_osHandle, &serialParams);
    if(!status){
        qCritical("Could not set the serial port parameters (failed to receive the configuration structure)");
        m_postInitParams.setState(TConfigParam::TState::TError, "Failed to set the post-init parameters");
        return m_postInitParams;
    }

    // Baudrate
    if(baudrateParam == "Custom"){
        DWORD baudrateTbs = customBaudrateParam.toULong();
        serialParams.BaudRate = baudrateTbs;
        qWarning(qPrintable(QString("Baudrate %1 support depends on the hardware and is generally not guaranteed!").arg(baudrateTbs)));
    } else {
        DWORD baudrateTbs = baudrateParam.toULong();

        switch (baudrateTbs) {

            case 110   : serialParams.BaudRate = CBR_110;    break;
            case 300   : serialParams.BaudRate = CBR_300;    break;
            case 600   : serialParams.BaudRate = CBR_600;    break;
            case 1200  : serialParams.BaudRate = CBR_1200;   break;
            case 2400  : serialParams.BaudRate = CBR_2400;   break;
            case 4800  : serialParams.BaudRate = CBR_4800;   break;
            case 9600  : serialParams.BaudRate = CBR_9600;   break;
            case 19200 : serialParams.BaudRate = CBR_19200;  break;
            case 38400 : serialParams.BaudRate = CBR_38400;  break;
            case 57600 : serialParams.BaudRate = CBR_57600;  break;
            case 115200: serialParams.BaudRate = CBR_115200; break;
            case 128000: serialParams.BaudRate = CBR_128000; break;
            case 256000: serialParams.BaudRate = CBR_256000; break;
            default:
                if(baudrateTbs > 0) {
                    serialParams.BaudRate = baudrateTbs;
                    qWarning(qPrintable(QString("Baudrate %1 support depends on the hardware and is generally not guaranteed!").arg(baudrateTbs)));
                } else {
                    qCritical("Unsupported baud rate. Should never get here.");
                }
                break;

        }
    }

    // Parity
    if (parityParam == "None") {

        serialParams.fParity = FALSE;
        serialParams.Parity = NOPARITY;

    } else {

        serialParams.fParity = TRUE;

        if (parityParam == "Even") {

            serialParams.Parity = EVENPARITY;

        } else { // ODD

            serialParams.Parity = ODDPARITY;

        }

    }

    // Stop bits
    if (stopbitsParam == "Two") {

        serialParams.StopBits = TWOSTOPBITS;

    } else { // ONE

        serialParams.StopBits = ONESTOPBIT;

    }

    // no flow control
    serialParams.fDtrControl = DTR_CONTROL_DISABLE;
    serialParams.fRtsControl = DTR_CONTROL_DISABLE;
    serialParams.fOutX = FALSE;
    serialParams.fInX = FALSE;
    serialParams.fOutxCtsFlow = FALSE;
    serialParams.fOutxDsrFlow = FALSE;
    serialParams.fDsrSensitivity = FALSE;

    serialParams.ByteSize = 8; // only support 8-bit word

    serialParams.fBinary = TRUE; // non-binary transfers are not supported on Win anyway, according to msdn
    serialParams.fErrorChar = FALSE; // no corrections
    serialParams.fNull = FALSE; // dont discard null chars
    serialParams.fAbortOnError = FALSE; // dont abort on errors

    status = SetCommState(m_osHandle, &serialParams);
    if (!status){
        m_postInitParams.setState(TConfigParam::TState::TError, "Failed to set the post-init parameters");
        qCritical("Failed to set the serial port parameters");
    }

#else

    int status;

    struct termios serialParams;

    status = tcgetattr(m_osHandle, &serialParams);
    if(status){
        qCritical("Could not set the serial port parameters (failed to receive the configuration structure)");
        m_postInitParams.setState(TConfigParam::TState::TError, "Failed to set the post-init parameters");
        return m_postInitParams;
    }

    // Baudrate
    if(baudrateParam == "Custom"){
        qWarning(qPrintable(QString("Custom baudrate is not supported on Linux!")));
    } else {
        unsigned long baudrateTbs = baudrateParam.toULong();

        switch (baudrateTbs) {
            case 110   : status = cfsetispeed(&serialParams, B110);    status |= cfsetospeed(&serialParams, B110);    break;
            case 300   : status = cfsetispeed(&serialParams, B300);    status |= cfsetospeed(&serialParams, B300);    break;
            case 600   : status = cfsetispeed(&serialParams, B600);    status |= cfsetospeed(&serialParams, B600);    break;
            case 1200  : status = cfsetispeed(&serialParams, B1200);   status |= cfsetospeed(&serialParams, B1200);   break;
            case 2400  : status = cfsetispeed(&serialParams, B2400);   status |= cfsetospeed(&serialParams, B2400);   break;
            case 4800  : status = cfsetispeed(&serialParams, B4800);   status |= cfsetospeed(&serialParams, B4800);   break;
            case 9600  : status = cfsetispeed(&serialParams, B9600);   status |= cfsetospeed(&serialParams, B9600);   break;
            case 19200 : status = cfsetispeed(&serialParams, B19200);  status |= cfsetospeed(&serialParams, B19200);  break;
            case 38400 : status = cfsetispeed(&serialParams, B38400);  status |= cfsetospeed(&serialParams, B38400);  break;
            case 57600 : status = cfsetispeed(&serialParams, B57600);  status |= cfsetospeed(&serialParams, B57600);  break;
            case 115200: status = cfsetispeed(&serialParams, B115200); status |= cfsetospeed(&serialParams, B115200); break;
            default:
                m_postInitParams.getSubParamByName("Baudrate")->setState(TConfigParam::TState::TError, "Unsupported baudrate");
                qCritical("Unsupported baud rate");
                break;
            }
    }

    if(status){
        m_postInitParams.getSubParamByName("Baudrate")->setState(TConfigParam::TState::TError, "Could not set input/output baudrate");
        qCritical("Could not set input/output baudrate");
    }

    if (parityParam == "None") {

        serialParams.c_cflag &= ~PARENB;
        serialParams.c_cflag &= ~PARODD;

    } else {

        serialParams.c_cflag |= PARENB;

        if (parityParam == "Even") {

            serialParams.c_cflag &= ~PARODD;

        } else { // ODD

            serialParams.c_cflag |= PARODD;

        }

    }

    if (stopbitsParam == "Two") {

        serialParams.c_cflag |= CSTOPB;

    } else { // ONE

        serialParams.c_cflag &= ~CSTOPB;

    }

    // No flow control
    serialParams.c_cflag &= ~CRTSCTS;
    serialParams.c_iflag &= ~(IXON | IXOFF | IXANY);

    serialParams.c_cflag &= ~CSIZE;
    serialParams.c_cflag |= CS8;    // only support 8-bit words
    serialParams.c_cflag |= (CLOCAL | CREAD);  // enable receiver

    serialParams.c_iflag |= (IGNPAR | IGNBRK); // ignore parity errors and break conditions
    serialParams.c_iflag &= ~(BRKINT | ISTRIP | INLCR | IGNCR | ICRNL | PARMRK | INPCK); // dont process the input
// UICLC is not in POSIX, so check first
#ifdef IUCLC
    serialParams.c_iflag &= ~IUCLC;
#endif

    serialParams.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG); // only raw input
#ifdef IEXTEN
    serialParams.c_lflag &= ~IEXTEN;
#endif

    serialParams.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR | OFILL); // dont process the output
#ifdef OLCUC
    serialParams.c_oflag &= ~OLCUC;
#endif
#ifdef ONOEOT
    serialParams.c_oflag &= ~ONOEOT;
#endif
#ifdef XTABS
    serialParams.c_oflag &= ~XTABS;
#endif
#ifdef OXTABS
    serialParams.c_oflag &= ~OXTABS;
#endif
    serialParams.c_oflag &= ~OPOST; // only raw output

    status = tcflush(m_osHandle, TCIFLUSH);
    if(status){
        qCritical("Could not flush the serial port");
    }

    status = tcsetattr(m_osHandle, TCSANOW, &serialParams);
    if(status){
        qCritical("Could not set serial port parameters");
        m_postInitParams.setState(TConfigParam::TState::TError, "Failed to set the serial port parameters");
    }

#endif

    // Timeout

#ifdef _WIN32

    COMMTIMEOUTS serialTimeouts = { 0 }; // in milliseconds

    serialTimeouts.ReadIntervalTimeout = 0; // no interval timeout

    serialTimeouts.ReadTotalTimeoutConstant = (DWORD) m_readTimeout;	// set only total timeouts
    serialTimeouts.ReadTotalTimeoutMultiplier = 0;

    serialTimeouts.WriteTotalTimeoutConstant = (DWORD) m_writeTimeout;
    serialTimeouts.WriteTotalTimeoutMultiplier = 0;

    status = SetCommTimeouts(m_osHandle, &serialTimeouts);
    if (!status){
        qCritical("Could not set the serial port timeouts");
        m_postInitParams.getSubParamByName("Read timeout (ms)")->setState(TConfigParam::TState::TError, "Failed to set the timeout.");
        m_postInitParams.getSubParamByName("Write timeout (ms)")->setState(TConfigParam::TState::TError, "Failed to set the timeout.");
    }

#else

    struct termios serialTimeout;

    status = tcgetattr(m_osHandle, &serialTimeout);
    if(status){
        qCritical("Could not get serial port parameters in order to set a timeout");
        m_postInitParams.getSubParamByName("Timeout (ms)")->setState(TConfigParam::TState::TError, "Failed to set the timeout.");
    }

    cc_t ds = m_readTimeout / 100; // POSIX timeouted read takes deciseconds
    if( ( m_readTimeout % 100 ) >= 50 || (ds == 0 && m_readTimeout > 0) ) ds++; // round up; or set the lowest timeout possible if asking for less, unless setting no timeout at all

    serialTimeout.c_cc[VTIME] = ds;
    serialTimeout.c_cc[VMIN] = 0;

    status = tcflush(m_osHandle, TCIFLUSH);
    if(status){
        qCritical("Could not flush the serial port while setting the timeout");
    }

    status = tcsetattr(m_osHandle, TCSANOW, &serialTimeout);
    if(status){
        qCritical("Could not set serial port timeout");
        m_postInitParams.getSubParamByName("Timeout (ms)")->setState(TConfigParam::TState::TError, "Failed to set the timeout.");
    }

#endif

    return m_postInitParams;

}

size_t TSerialPortDevice::writeData(const uint8_t * buffer, size_t len){

#ifdef _WIN32

    DWORD bytesWritten = 0;
    BOOL status;

    status = WriteFile(m_osHandle, buffer, len, &bytesWritten, NULL);
    if (!status){
        qCritical("Write to the serial port failed");
        return 0;
    }

    if(len != bytesWritten){
        qCritical("Serial port write timeout.");
    }

    return (size_t) bytesWritten;

#else

    ssize_t bytesWritten = write(m_osHandle, (const void *) buffer, len);
    if(bytesWritten < 0){
        qCritical("Write to the serial port failed");
        return 0;
    }

    if(len != bytesWritten){
        qCritical("Serial port write timeout.");
    }

    return (size_t) bytesWritten;

#endif

}

size_t TSerialPortDevice::readData(uint8_t * buffer, size_t len) {

#ifdef _WIN32

    DWORD bytesRead = 0;
    BOOL status;

    // ReadFile returns either when all the requested bytes have been read, or the timeout expires
    // (wont return with less data than required, unless the timeout expires)

    status = ReadFile(m_osHandle, buffer, len, &bytesRead, NULL);
    if (!status){
        qCritical("Read from the serial port failed");
        return 0;
    }

    //if(len != bytesRead) qWarning("Serial port read timeout.");

    return (size_t) bytesRead;

#else

    size_t bytesRead = 0;
    ssize_t readRet = 1; // set to allow the loop to start

    while(bytesRead < len && readRet > 0){

        // read returns either when any data is available, or when the timeout expires
        // (read may return immediately with a single byte of data available, wont wait for the rest of the requested data; hence the loop)
        // (actually, it might wait, it is really implementation dependent; prefer a posix compliant way)

        readRet = read(m_osHandle, (void *)(buffer + bytesRead), len - bytesRead);

        if(readRet < 0) {

            qCritical("Read from the serial port failed");
            return 0;

        } else {

            bytesRead += readRet;

        }

    }

    //if(len != bytesRead) qWarning("Serial port read timeout.");

    return bytesRead;

#endif

}

std::optional<size_t> TSerialPortDevice::availableBytes(){
    return std::nullopt;
}

