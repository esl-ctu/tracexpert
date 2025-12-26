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
// Adam Švehla (initial author)

#include "trandom.h"

TRandom::TRandom(): m_randomGenerators(), m_preInitParams(), m_postInitParams() { }

TRandom::~TRandom() {

    (*this).TRandom::deInit();
}

QString TRandom::getName() const {
    return QString("Random number generator");
}

QString TRandom::getInfo() const {
    return QString("Provides random number generation.");
}

TConfigParam TRandom::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TRandom::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TRandom::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TRandom::deInit(bool *ok) {
    qDeleteAll(m_randomGenerators.begin(), m_randomGenerators.end());
    m_randomGenerators.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TRandom::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TRandom::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TRandom::addIODevice(QString name, QString info, bool *ok) {
    TIODevice * ret = new TRandomDevice(name, info);
    m_randomGenerators.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TScope * TRandom::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TRandom::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TRandom::canAddIODevice() {
    return true;
}

bool TRandom::canAddScope() {
    return false;
}

bool TRandom::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TRandom::getIODevices() {
    return m_randomGenerators;
}

QList<TScope *> TRandom::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TRandom::getAnalDevices() {
    return QList<TAnalDevice *>();
}
