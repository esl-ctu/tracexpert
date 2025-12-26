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

#ifndef TPS6000SCOPE_H
#define TPS6000SCOPE_H

#include "tscope.h"
#include <ps6000Api.h>
#include <QThread>

class TPS6000Scope : public TScope {

public:

    TPS6000Scope(const QString & name, const QString & info);

    virtual ~TPS6000Scope() override;

    virtual QString getName() const override;
    virtual QString getInfo() const override;

    virtual TConfigParam getPreInitParams() const override;
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    virtual void init(bool *ok = nullptr) override;
    virtual void deInit(bool *ok = nullptr) override;

    virtual TConfigParam getPostInitParams() const override;
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual void run(size_t * expectedBufferSize, bool *ok = nullptr) override;
    virtual void stop(bool *ok = nullptr) override;

    /// Downloads samples from the oscilloscope, first waits for the aquisition to complete. Works with a char memory buffer, fills it with an array of samplesType values (typically will need a recast!). Returns size in bytes.
    virtual size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage) override;

    virtual QList<TScope::TChannelStatus> getChannelsStatus() override;
    virtual TScope::TTimingStatus getTimingStatus() override;
    virtual TScope::TTriggerStatus getTriggerStatus() override;


protected:

    void _init(bool *ok = nullptr, bool createParams = true);
    void _createPostInitParams();
    void _setChannels();
    void _setTrigger();
    void _setTiming();
    qreal rangeStrToReal(const QString & str);

    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;
    bool m_running;
    int16_t m_handle;

    uint32_t m_preTrigSamples;
    uint32_t m_postTrigSamples;
    uint32_t m_captures;
    uint32_t m_timebase;   
    qreal m_samplingPeriod;
    bool m_channelEnabled;

};

#endif // TPS6000SCOPE_H
