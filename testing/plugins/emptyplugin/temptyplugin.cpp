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

#include "temptyplugin.h"

TEmptyPlugin::TEmptyPlugin() {
    
}

TEmptyPlugin::~TEmptyPlugin() {

}

QString TEmptyPlugin::getName() const {
    return QString("Empty plugin");
}

QString TEmptyPlugin::getInfo() const {
    return QString("Empty plugin info");
}


TConfigParam TEmptyPlugin::getPreInitParams() const {
    return TConfigParam();
}

TConfigParam TEmptyPlugin::setPreInitParams(TConfigParam params) {
    return TConfigParam();
}

void TEmptyPlugin::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TEmptyPlugin::deInit(bool *ok) {
    if(ok != nullptr) *ok = true;
}

TConfigParam TEmptyPlugin::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TEmptyPlugin::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TEmptyPlugin::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = true;
    return nullptr;
}

TScope * TEmptyPlugin::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TEmptyPlugin::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TEmptyPlugin::canAddIODevice() {
    return true;
}

bool TEmptyPlugin::canAddScope() {
    return false;
}

bool TEmptyPlugin::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TEmptyPlugin::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TEmptyPlugin::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TEmptyPlugin::getAnalDevices() {
    return QList<TAnalDevice *>();
}
