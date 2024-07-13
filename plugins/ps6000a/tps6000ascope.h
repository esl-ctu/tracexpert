#ifndef TPS6000aSCOPE_H
#define TPS6000aSCOPE_H

#include "tscope.h"
#include <ps6000aApi.h>

#include <QThread>

#define PICO_10MV PICO_X1_PROBE_10MV
#define PICO_20MV PICO_X1_PROBE_20MV
#define PICO_50MV PICO_X1_PROBE_50MV
#define PICO_100MV PICO_X1_PROBE_100MV
#define PICO_200MV PICO_X1_PROBE_200MV
#define PICO_500MV PICO_X1_PROBE_500MV
#define PICO_1V PICO_X1_PROBE_1V
#define PICO_2V PICO_X1_PROBE_2V
#define PICO_5V PICO_X1_PROBE_5V
#define PICO_10V PICO_X1_PROBE_10V
#define PICO_20V PICO_X1_PROBE_20V

class TPS6000aScope : public TScope {

public:

    TPS6000aScope(const QString & name, const QString & info);

    virtual ~TPS6000aScope() override;

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

    void _createPostInitParams();
    void _setChannels();
    void _setTrigger();
    void _setTiming();
    void _getChannelsAndResolution(uint8_t * channels, uint8_t * resolution, uint32_t * bandwidth);
    uint32_t _getTimebase(qreal samplingPeriod);
    qreal _getSamplingPeriod(uint32_t timeBase);
    qreal rangeStrToReal(const QString & str);

    QString m_name;
    QString m_info;
    QString m_model;
    uint16_t m_resolution;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;
    bool m_running;
    int16_t m_handle;

    uint32_t m_preTrigSamples;
    uint32_t m_postTrigSamples;
    uint64_t m_captures;
    uint32_t m_timebase;   
    qreal m_samplingPeriod;

};

#endif // TPS6000aSCOPE_H
