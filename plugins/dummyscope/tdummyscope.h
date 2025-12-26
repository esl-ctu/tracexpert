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

#ifndef TDUMMYSCOPE_H
#define TDUMMYSCOPE_H

#include <QThread>
#include <QString>
#include <QRandomGenerator>
#include "qdebug.h"
#include "tconfigparam.h"
#include "tscope.h"


class TDummyScope : public TScope {

public:
    TDummyScope() {
        m_postInitParams = TConfigParam("Dummy scope configuration", "", TConfigParam::TType::TDummy, "");
        m_postInitParams.addSubParam(TConfigParam("Channel 1 enabled", "true", TConfigParam::TType::TBool, ""));
        m_postInitParams.addSubParam(TConfigParam("Channel 2 enabled", "true", TConfigParam::TType::TBool, ""));
        m_postInitParams.addSubParam(TConfigParam("Channel 3 enabled", "true", TConfigParam::TType::TBool, ""));
        m_postInitParams.addSubParam(TConfigParam("Channel 4 enabled", "true", TConfigParam::TType::TBool, ""));

        m_postInitParams.addSubParam(TConfigParam("Channel 1 offset", "0", TConfigParam::TType::TReal, ""));
        m_postInitParams.addSubParam(TConfigParam("Channel 2 offset", "0", TConfigParam::TType::TReal, ""));
        m_postInitParams.addSubParam(TConfigParam("Channel 3 offset", "0", TConfigParam::TType::TReal, ""));
        m_postInitParams.addSubParam(TConfigParam("Channel 4 offset", "0", TConfigParam::TType::TReal, ""));
    }

    ~TDummyScope() {}

    /// Scope name
    QString getName() const {
        return "Dummy scope";
    }

    /// Scope info
    QString getInfo() const {
        return "Dummy scope for testing purposes.";
    }

    /// Get the current pre-initialization parameters
    TConfigParam getPreInitParams() const {
        return TConfigParam();
    }

    /// Set the pre-initialization parameters, returns the current params after set
    TConfigParam setPreInitParams(TConfigParam params) {
        return params;
    }

    /// Initialize the scope
    void init(bool *ok = nullptr) {
        if(ok != nullptr) {
            *ok = true;
        }
    }
    /// Deinitialize the scope
    virtual void deInit(bool *ok = nullptr) {
        if(ok != nullptr) {
            *ok = true;
        }
    }



    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const {
        return m_postInitParams;
    }
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) {
        m_postInitParams = params;
        return params;
    }

    bool m_enabled = false;

    /// Run the oscilloscope: wait for trigger when set, otherwise capture immediately
    void run(size_t * expectedBufferSize, bool *ok = nullptr) {
        *expectedBufferSize = 500000*4;

        if(ok != nullptr) {
            *ok = true;
        }

        m_enabled = true;
    }
    /// Stop the oscilloscope
    void stop(bool *ok = nullptr) {
        if(ok != nullptr) {
            *ok = true;
        }

        m_enabled = false;
    }

    /// Downloads samples from the oscilloscope, first waits for the aquisition to complete. Works with a char memory buffer,
    /// fills it with an array of samplesType values (typically will need a recast!). Returns size in bytes.
    // size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType & samplesType, size_t & samplesPerTraceDownloaded, size_t & tracesDownloaded) {
    size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage) {

        TConfigParam * enabledParam = m_postInitParams.getSubParamByName(QString("Channel %1 enabled").arg(channel+1));

        if(!m_enabled || !enabledParam || enabledParam->getValue() != "true") {

            *tracesDownloaded = 0;
            return 0;
        }

        //QThread::msleep(250);

        *samplesType = TSampleType::TInt32;
        *samplesPerTraceDownloaded = 500000;
        *tracesDownloaded = 1;

        int32_t * internalBuffer = (int32_t *)buffer;
        for (size_t i = 0; i < 500000; i++) {
            internalBuffer[i] = qSin(qDegreesToRadians(i%360))*1000 + (QRandomGenerator::global()->generate() % 500) - 250; //((int16_t)(QRandomGenerator::global()->generate() % 2000))-1000;
        }

        return 500000*4;
    }

    /// Get channel info
    QList<TChannelStatus> getChannelsStatus() {
        auto channelStatusList = QList<TChannelStatus>();
        channelStatusList.append(TChannelStatus(0, "A", m_postInitParams.getSubParamByName(QString("Channel 1 enabled"))->getValue() == "true",
                                                 5, m_postInitParams.getSubParamByName(QString("Channel 1 offset"))->getValue().toDouble(), -1000, 1000));
        channelStatusList.append(TChannelStatus(1, "B", m_postInitParams.getSubParamByName(QString("Channel 2 enabled"))->getValue() == "true",
                                                10, m_postInitParams.getSubParamByName(QString("Channel 2 offset"))->getValue().toDouble(), -1000, 1000));
        channelStatusList.append(TChannelStatus(2, "C", m_postInitParams.getSubParamByName(QString("Channel 3 enabled"))->getValue() == "true",
                                                15, m_postInitParams.getSubParamByName(QString("Channel 3 offset"))->getValue().toDouble(), -1000, 1000));
        channelStatusList.append(TChannelStatus(3, "D", m_postInitParams.getSubParamByName(QString("Channel 4 enabled"))->getValue() == "true",
                                                20, m_postInitParams.getSubParamByName(QString("Channel 4 offset"))->getValue().toDouble(), -1000, 1000));
        return channelStatusList;
    }

    TTimingStatus getTimingStatus() {
        return TTimingStatus(0, 0, 0, 0);
    }

    TTriggerStatus getTriggerStatus() {
        return TTriggerStatus(TTriggerStatus::TTriggerType::TAbove, 20, 5);
    }

private:
    TConfigParam m_postInitParams;
};

#endif // TDUMMYSCOPE_H
