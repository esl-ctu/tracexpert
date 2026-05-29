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

#ifndef FILEDEVICE_H
#define FILEDEVICE_H

#include <QFileInfo>
#include <QFile>
#include <QElapsedTimer>
#include "tiodevice.h"

class TFile;

class TFileDevice : public TIODevice {

public:

    TFileDevice(QString & name, QString & info, TFile & tFile);

    virtual ~TFileDevice() override;

    virtual QString getName() const override;
    virtual QString getInfo() const override;

    virtual TConfigParam getPreInitParams() const override;
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    virtual void init(bool *ok = nullptr) override;
    virtual void deInit(bool *ok = nullptr) override;

    virtual TConfigParam getPostInitParams() const override;
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual size_t writeData(const uint8_t * buffer, size_t len) override;
    virtual size_t readData(uint8_t * buffer, size_t len) override;
    virtual std::optional<size_t> availableBytes() override;

protected:

    void _openFile(bool *ok = nullptr);
    void _createPreInitParams();
    void _createPostInitParams();
    bool _validatePreInitParamsStructure(TConfigParam & params);
    bool _validatePostInitParamsStructure(TConfigParam & params);

    bool m_createdManually;
    QFlags<QIODevice::OpenModeFlag> m_openMode;
    QFile m_file;
    QFileInfo m_fileInfo;
    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;

    TFile & m_tFile;
};

#endif // FILEDEVICE_H
