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

TAnalDevice * TSerialPort::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}


bool TSerialPort::canAddIODevice() {
    return true;
}

bool TSerialPort::canAddScope() {
    return false;
}
bool TSerialPort::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TSerialPort::getIODevices() {
    return m_ports;
}

QList<TScope *> TSerialPort::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TSerialPort::getAnalDevices(){
    return QList<TAnalDevice *>();
}
