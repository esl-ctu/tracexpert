#include "tserialport.h"

TSerialPort::TSerialPort(): m_ports(), m_preInitParams(), m_postInitParams() {
    m_preInitParams  = TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect serial ports available", false);
}

TSerialPort::~TSerialPort() {

    (*this).TSerialPort::deInit();
}

QString TSerialPort::getName() const {
    return QString("Serial port");
}

QString TSerialPort::getInfo() const {
    return QString("Provides access to local serial ports.");
}


TConfigParam TSerialPort::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TSerialPort::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TSerialPort::init(bool *ok) {

    if(m_preInitParams.getName() == "Auto-detect" && m_preInitParams.getValue() == "true") { // if auto-detect is enabled
        const auto serialPortInfos = QSerialPortInfo::availablePorts();
        for (const QSerialPortInfo &portInfo : serialPortInfos) {
            m_ports.append(new TSerialPortDevice(portInfo));
        }
    }
    if(ok != nullptr) *ok = true;
}

void TSerialPort::deInit(bool *ok) {
    qDeleteAll(m_ports.begin(), m_ports.end());
    m_ports.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TSerialPort::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TSerialPort::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TSerialPort::addIODevice(QString name, QString info, bool *ok) {
    TIODevice * ret = new TSerialPortDevice(name, info);
    m_ports.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TScope * TSerialPort::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TSerialPort::canAddIODevice() {
    return true;
}

bool TSerialPort::canAddScope() {
    return false;
}

QList<TIODevice *> TSerialPort::getIODevices() {
    return m_ports;
}

QList<TScope *> TSerialPort::getScopes() {
    return QList<TScope *>();
}
