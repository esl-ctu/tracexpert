// COPYRIGHT HEADER BEGIN
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
// COPYRIGHT HEADER END

#include "tnewplugin.h"

TNewPlugin::TNewPlugin() {
    
}

TNewPlugin::~TNewPlugin() {

}

QString TNewPlugin::getName() const {
    return QString("New plugin");
}

QString TNewPlugin::getInfo() const {
    return QString("New plugin info");
}


TConfigParam TNewPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TNewPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TNewPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TNewPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TNewPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TNewPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TNewPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TNewPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TNewPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TNewPlugin::canAddIODevice() {
    return true;
}

bool TNewPlugin::canAddScope() {
    return false;
}

bool TNewPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TNewPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TNewPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TNewPlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
