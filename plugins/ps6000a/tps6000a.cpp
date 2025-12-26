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

#include "tps6000a.h"

TPS6000a::TPS6000a(): m_scopes(), m_preInitParams() {
    m_preInitParams  = TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect Picoscope 6000E series scopes available", false);
}

TPS6000a::~TPS6000a() {
    (*this).TPS6000a::deInit();
}

QString TPS6000a::getName() const {
    return QString("Picoscope 6000 E series");
}

QString TPS6000a::getInfo() const {
    return QString("Provides access to Picoscope 6000 E series oscilloscopes");
}


TConfigParam TPS6000a::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TPS6000a::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TPS6000a::init(bool *ok) {
    bool iok = true;
    if(m_preInitParams.getName() == "Auto-detect" && m_preInitParams.getValue() == "true") { // if auto-detect is enabled

        int16_t count;
        std::unique_ptr<int8_t[]> serials(new int8_t[1024]);
        int16_t serials_len = 1024;

        PICO_STATUS status = ps6000aEnumerateUnits(&count, serials.get(), &serials_len);

        if(status == PICO_OK){

            QString qserials((char*) serials.get());
            QStringList serialsList = qserials.split(QLatin1Char(','));

            if (count == 0) { // no scope found
                iok = true;
            } else if(serialsList.size() == count) { // scopes found

                for (const QString &serialNo : serialsList) {

                    m_scopes.append(new TPS6000aScope(serialNo, "Automatically detected"));

                }

            } else {
                qWarning("Picoscope auto-detection went wrong. This should never happen.");
                iok = false;
            }

        } else if(status == PICO_NOT_FOUND) { // no scope found
            iok = true;
        } else {
            qWarning("Picoscope auto-detection went wrong. Either the API is busy, or there is a hardware failure.");
            iok = false;
        }
    }
    if(ok != nullptr){
        *ok = iok; // TODO iok setup
    }
}

void TPS6000a::deInit(bool *ok) {
    qDeleteAll(m_scopes.begin(), m_scopes.end());
    m_scopes.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TPS6000a::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TPS6000a::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TPS6000a::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TPS6000a::addScope(QString name, QString info, bool *ok) {
    TScope * ret = new TPS6000aScope(name, info);
    m_scopes.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TAnalDevice * TPS6000a::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}


bool TPS6000a::canAddIODevice() {
    return false;
}

bool TPS6000a::canAddScope() {
    return true;
}
bool TPS6000a::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TPS6000a::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TPS6000a::getScopes() {
    return m_scopes;
}

QList<TAnalDevice *> TPS6000a::getAnalDevices(){
    return QList<TAnalDevice *>();
}
