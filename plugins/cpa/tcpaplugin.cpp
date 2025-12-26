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

#include "tcpaplugin.h"

#include "tcpadevice.h"

TCPAPlugin::TCPAPlugin() {
    
}

TCPAPlugin::~TCPAPlugin() {
    (*this).TCPAPlugin::deInit();
}

QString TCPAPlugin::getName() const {
    return QString("Correlation power analysis");
}

QString TCPAPlugin::getInfo() const {
    return QString("Component for univariate correlation power analysis");
}


TConfigParam TCPAPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TCPAPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TCPAPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TCPADevice());
}

void TCPAPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TCPAPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TCPAPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TCPAPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TCPAPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TCPAPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TCPAPlugin::canAddIODevice() {
    return false;
}

bool TCPAPlugin::canAddScope() {
    return false;
}

bool TCPAPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TCPAPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TCPAPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TCPAPlugin::getAnalDevices() {
    return m_analDevices;
}

