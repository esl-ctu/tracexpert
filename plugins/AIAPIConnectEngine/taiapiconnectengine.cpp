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
// Tomáš Přeučil (initial author)

#include "taiapiconnectengine.h"

TAIAPIConnectEngine::TAIAPIConnectEngine() {
    qDebug("eee");
    m_preInitParams  = TConfigParam("AI API connect engine pre-init", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("Nothing to do here, click next please", "", TConfigParam::TType::TDummy, ""));
    m_postInitParams  = TConfigParam("AI API connect engine post-init", "", TConfigParam::TType::TDummy, "");
    m_postInitParams.addSubParam(TConfigParam("Nothing to do here, click next please", "", TConfigParam::TType::TDummy, ""));
    m_initialized = false;
}

TAIAPIConnectEngine::~TAIAPIConnectEngine() {}

QString TAIAPIConnectEngine::getName() const {
    return QString("AI API Conect Engine (beta)");
}

QString TAIAPIConnectEngine::getInfo() const {
    return QString("Connects to a python AI server");
}

TConfigParam TAIAPIConnectEngine::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TAIAPIConnectEngine::setPreInitParams(TConfigParam params) {
    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

void TAIAPIConnectEngine::init(bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = true;
    m_initialized = true;
}

void TAIAPIConnectEngine::deInit(bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = true;
    m_initialized = false;

    for (int i = 0; i < m_analDevices.length(); i++) {
        delete m_analDevices[i];
    }
    m_analDevices.clear();

}

TConfigParam TAIAPIConnectEngine::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TAIAPIConnectEngine::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TAIAPIConnectEngine::addIODevice(QString name, QString info, bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TAIAPIConnectEngine::addScope(QString name, QString info, bool *ok /*= nullptr*/) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TAIAPIConnectEngine::addAnalDevice(QString name, QString info, bool *ok /*= nullptr*/) {
    TAnalDevice * ret = new TAIAPIConnectEngineDevice(name, info);
    m_analDevices.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;

}

bool TAIAPIConnectEngine::canAddIODevice() {
    return false;
}

bool TAIAPIConnectEngine::canAddScope() {
    return false;
}

bool TAIAPIConnectEngine::canAddAnalDevice() {
    return true;
}

QList<TIODevice *> TAIAPIConnectEngine::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TAIAPIConnectEngine::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TAIAPIConnectEngine::getAnalDevices() {
    return m_analDevices;
}
