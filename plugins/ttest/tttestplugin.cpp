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

#include "tttestplugin.h"

#include "tttestdevice.h"

TTTestPlugin::TTTestPlugin() {
    
}

TTTestPlugin::~TTTestPlugin() {
    (*this).TTTestPlugin::deInit();
}

QString TTTestPlugin::getName() const {
    return QString("Welch's t-test");
}

QString TTTestPlugin::getInfo() const {
    return QString("Component for t-test leakage assessment");
}


TConfigParam TTTestPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TTTestPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TTTestPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;

    m_analDevices.append(new TTTestDevice());
}

void TTTestPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();
}

TConfigParam TTTestPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TTTestPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TTTestPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TTTestPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TTTestPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TTTestPlugin::canAddIODevice() {
    return false;
}

bool TTTestPlugin::canAddScope() {
    return false;
}

bool TTTestPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TTTestPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TTTestPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TTTestPlugin::getAnalDevices() {
    return m_analDevices;
}
