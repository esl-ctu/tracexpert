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

#include "tdummyscopeplugin.h"
#include "tdummyscope.h"

TDummyScopePlugin::TDummyScopePlugin() {

}

TDummyScopePlugin::~TDummyScopePlugin() {

}

QString TDummyScopePlugin::getName() const {
    return QString("Dummy scope plugin");
}

QString TDummyScopePlugin::getInfo() const {
    return QString("Dummy scope for oscilloscope widget testing");
}


TConfigParam TDummyScopePlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TDummyScopePlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TDummyScopePlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TDummyScopePlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TDummyScopePlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TDummyScopePlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TDummyScopePlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TDummyScopePlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TDummyScopePlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TDummyScopePlugin::canAddIODevice() {
    return false;
}

bool TDummyScopePlugin::canAddScope() {
    return true;
}

bool TDummyScopePlugin::canAddAnalDevice()  {
    return false;
}

QList<TIODevice *> TDummyScopePlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TDummyScopePlugin::getScopes() {
    auto scopeList = QList<TScope *>();
    scopeList.append(new TDummyScope());
    return scopeList;
}

QList<TAnalDevice *> TDummyScopePlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
