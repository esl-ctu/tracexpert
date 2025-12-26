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

#include "tpredictplugin.h"

#include "tpredictaes.h"

TPredictPlugin::TPredictPlugin() {
    
}

TPredictPlugin::~TPredictPlugin() {
    (*this).TPredictPlugin::deInit();
}

QString TPredictPlugin::getName() const {
    return QString("Leakage predictions");
}

QString TPredictPlugin::getInfo() const {
    return QString("Plugin computing leakage predictions");
}


TConfigParam TPredictPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TPredictPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TPredictPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TPredictAES());
}

void TPredictPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TPredictPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TPredictPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TPredictPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TPredictPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TPredictPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TPredictPlugin::canAddIODevice() {
    return false;
}

bool TPredictPlugin::canAddScope() {
    return false;
}

bool TPredictPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TPredictPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TPredictPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TPredictPlugin::getAnalDevices() {
    return m_analDevices;
}

