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

#ifndef SMARTCARDDEVICE_H
#define SMARTCARDDEVICE_H

#include <QElapsedTimer>
#include "tiodevice.h"

#include <QQueue>
#include <winscard.h>

class TSmartCardDevice : public TIODevice {

public:

    TSmartCardDevice(QString & name, QString & info);
    TSmartCardDevice(QString & mszReader);

    virtual ~TSmartCardDevice() override;

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

    void _createPostInitParams();
    bool _validatePostInitParamsStructure(TConfigParam & params);

    bool m_createdManually;
    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;
    bool m_inputCreateAPDU;
    bool m_outputParseAPDU;
    unsigned char m_APDUHeader[4];
    unsigned char m_APDUTrailer;
    QQueue<QByteArray> m_recQueue;
    SCARDCONTEXT m_context;
    SCARDHANDLE m_card;

};

#endif // SMARTCARDDEVICE_H
