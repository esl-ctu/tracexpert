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

#include "tcipherplugin.h"

#include "taesengine.h"
#include "tpresentengine.h"

TCipherPlugin::TCipherPlugin() {
    
}

TCipherPlugin::~TCipherPlugin() {
    (*this).TCipherPlugin::deInit();
}

QString TCipherPlugin::getName() const {
    return QString("Ciphers");
}

QString TCipherPlugin::getInfo() const {
    return QString("Computes encryption/decryption intermediate values");
}


TConfigParam TCipherPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TCipherPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TCipherPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TAESEngine());
    m_analDevices.append(new TPRESENTEngine());
}

void TCipherPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TCipherPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TCipherPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TCipherPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TCipherPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TCipherPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TCipherPlugin::canAddIODevice() {
    return false;
}

bool TCipherPlugin::canAddScope() {
    return false;
}

bool TCipherPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TCipherPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TCipherPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TCipherPlugin::getAnalDevices() {
    return m_analDevices;
}

