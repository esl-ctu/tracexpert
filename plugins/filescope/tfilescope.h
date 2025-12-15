#ifndef TFILESCOPE_H
#define TFILESCOPE_H

#include <QFile>
#include <QString>
#include "tconfigparam.h"
#include "tscope.h"


class TFileScope : public TScope {

public:
    TFileScope();

    /// Scope name
    QString getName() const;

    /// Scope info
    QString getInfo() const;

    /// Get the current pre-initialization parameters
    TConfigParam getPreInitParams() const;

    /// Set the pre-initialization parameters, returns the current params after set
    TConfigParam setPreInitParams(TConfigParam params);

    /// Initialize the scope
    void init(bool *ok = nullptr);
    /// Deinitialize the scope
    virtual void deInit(bool *ok = nullptr);


    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params);

    /// Run the oscilloscope: wait for trigger when set, otherwise capture immediately
    void run(size_t * expectedBufferSize, bool *ok = nullptr);
    /// Stop the oscilloscope
    void stop(bool *ok = nullptr);

    /// Downloads samples from the oscilloscope, first waits for the aquisition to complete. Works with a char memory buffer,
    /// fills it with an array of samplesType values (typically will need a recast!). Returns size in bytes.
    // size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType & samplesType, size_t & samplesPerTraceDownloaded, size_t & tracesDownloaded) {
    size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage);

    /// Get channel info
    QList<TChannelStatus> getChannelsStatus();

    TTimingStatus getTimingStatus();

    TTriggerStatus getTriggerStatus();

private:
    TSampleType getSampleType();
    int getTypeBytes();
    qsizetype getTypeMin();
    qsizetype getTypeMax();

    TConfigParam m_postInitParams;

    QFile m_file;
};

#endif // TFILESCOPE_H
