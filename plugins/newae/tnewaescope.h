#ifndef NEWAESCOPE_H
#define NEWAESCOPE_H
#pragma once

#include "tnewae_global.h"
#include <QTimer>

//#include "tnewae.h"

class TNewae;

class TnewaeScope: public TScope {
public:
    //Should never be called directly. If you need to create a scope manually, use the addScope() function from Tnewae and make sure that the info string is empty!
    TnewaeScope(const QString & name_in, const QString & info_in, uint8_t id_in, TNewae * plugin_in, bool createdManually_in = true);
    virtual ~TnewaeScope();

    virtual QString getName() const override;
    virtual QString getInfo() const override;
    QString getScopeSn() const;

    virtual TConfigParam getPreInitParams() const override;
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    virtual void init(bool *ok = nullptr) override;
    virtual void deInit(bool *ok = nullptr) override;

    virtual TConfigParam getPostInitParams() const override;
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    /// Run the oscilloscope: wait for trigger when triggered, otherwise capture immediately
    virtual void run(size_t * expectedBufferSize, bool *ok = nullptr) override;
    /// Stop the oscilloscope
    virtual void stop(bool *ok = nullptr) override;

    /// Downloads values from the oscilloscope, first waits for the aquisition to complete
    virtual size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize,
                                   TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded , bool * overvoltage) override;

    /// Get channel info
    virtual QList<TChannelStatus> getChannelsStatus() override;
    virtual TTimingStatus getTimingStatus() override;
    virtual TTriggerStatus getTriggerStatus() override;

    uint8_t getId();
    QString getSn();
    void notConnectedError();
    bool isInitialized();

protected:
    QString sn;
    uint8_t cwId;
    QString name;
    QString info;
    TNewae * plugin;
    bool running;

    QList<TChannelStatus> chanStatus;
    bool m_createdManually;

    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    qint32 m_readTimeout;
    qint32 m_writeTimeout;
    bool m_initialized;
    bool traceWaitingForRead;
    int cwBufferSize;
    bool stopNow;

    //void _createPreInitParams();
    TConfigParam _createPostInitParams();
    bool _validatePreInitParamsStructure(TConfigParam & params);
    bool _validatePostInitParamsStructure(TConfigParam & params);
    TConfigParam updatePostInitParams(TConfigParam paramsIn, bool write = false) const;
};

#endif //NEWAESCOPE_H

