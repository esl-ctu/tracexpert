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
// Vojtěch Miškovský (initial author)

#include "tfilescopeplugin.h"
#include "tfilescope.h"

TFileScopePlugin::TFileScopePlugin() {

}

TFileScopePlugin::~TFileScopePlugin() {

}

QString TFileScopePlugin::getName() const {
    return QString("File scope plugin");
}

QString TFileScopePlugin::getInfo() const {
    return QString("Plugin for loading and displaying traces from file");
}


TConfigParam TFileScopePlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TFileScopePlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TFileScopePlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TFileScopePlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TFileScopePlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TFileScopePlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TFileScopePlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TFileScopePlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TFileScopePlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TFileScopePlugin::canAddIODevice() {
    return false;
}

bool TFileScopePlugin::canAddScope() {
    return true;
}

bool TFileScopePlugin::canAddAnalDevice()  {
    return false;
}

QList<TIODevice *> TFileScopePlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TFileScopePlugin::getScopes() {
    auto scopeList = QList<TScope *>();
    scopeList.append(new TFileScope());
    return scopeList;
}

QList<TAnalDevice *> TFileScopePlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
