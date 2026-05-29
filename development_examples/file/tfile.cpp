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
// Petr Socha

#include "tfile.h"

TFile::TFile(): m_files(), m_preInitParams(), m_postInitParams() { }

TFile::~TFile() {
    deInit();
}

QString TFile::getName() const {
    return QString("File");
}

QString TFile::getInfo() const {
    return QString("Provides access to files.");
}


TConfigParam TFile::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TFile::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TFile::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TFile::deInit(bool *ok) {
    qDeleteAll(m_files.begin(), m_files.end());
    m_files.clear();

    if(ok != nullptr) *ok = true;
}

TConfigParam TFile::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TFile::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TFile::addIODevice(QString name, QString info, bool *ok) {
    TIODevice * ret = new TFileDevice(name, info, *this);
    m_files.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TScope * TFile::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TAnalDevice * TFile::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TFile::canAddIODevice() {
    return true;
}

bool TFile::canAddScope() {
    return false;
}

bool TFile::canAddAnalDevice()  {
    return false;
}

QList<TIODevice *> TFile::getIODevices() {
    return m_files;
}

QList<TScope *> TFile::getScopes() {
    return QList<TScope *>();
}

QList<TAnalDevice *> TFile::getAnalDevices() {
    return QList<TAnalDevice *>();
}

bool TFile::registerOpenFile(std::filesystem::path path) {
    if(m_openFilePaths.contains(path)) {
        return false;
    }
    else {
        m_openFilePaths.append(path);
        return true;
    }
}

void TFile::unregisterOpenFile(std::filesystem::path path) {
    m_openFilePaths.removeAll(path);
}

