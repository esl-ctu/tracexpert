#ifndef TDUMMYSCOPE_H
#define TDUMMYSCOPE_H

#include <QThread>
#include <QString>
#include <QRandomGenerator>
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
        *expectedBufferSize = 1000*2;

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

        QThread::msleep(250);

        *samplesType = TSampleType::TUInt16;
        *samplesPerTraceDownloaded = 1000;
        *tracesDownloaded = 1;

        int16_t * internalBuffer = (int16_t *)buffer;
        for (size_t i = 0; i < 1000; i++) {
            internalBuffer[i] = QRandomGenerator::global()->generate() % 65535;
        }

        return 1000*2;
    }

    /// Get channel info
    QList<TChannelStatus> getChannelsStatus() {
        auto channelStatusList = QList<TChannelStatus>();
        channelStatusList.append(TChannelStatus(0, "A", m_postInitParams.getSubParamByName(QString("Channel 1 enabled"))->getValue() == "true",  5, 0));
        channelStatusList.append(TChannelStatus(1, "B", m_postInitParams.getSubParamByName(QString("Channel 2 enabled"))->getValue() == "true", 10, 0));
        channelStatusList.append(TChannelStatus(2, "C", m_postInitParams.getSubParamByName(QString("Channel 3 enabled"))->getValue() == "true", 15, 0));
        channelStatusList.append(TChannelStatus(3, "D", m_postInitParams.getSubParamByName(QString("Channel 4 enabled"))->getValue() == "true", 20, 0));
        return channelStatusList;
    }

private:
    TConfigParam m_postInitParams;
};

#endif // TDUMMYSCOPE_H
