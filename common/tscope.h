#ifndef TSCOPE_H
#define TSCOPE_H

#include <QString>
#include "tconfigparam.h"
#include "tcommon.h"

class TScope : public TCommon {

public:  

    enum class TSampleType {
        TUInt8,
        TInt8,
        TUInt16,
        TInt16,
        TUInt32,
        TInt32,
        TReal32,
        TReal64
    };

    class TChannelStatus {
    public:
        TChannelStatus(int index, QString alias, bool enabled, qreal range, qreal offset): m_index(index), m_alias(alias), m_enabled(enabled), m_range(range), m_offset(offset) {}
        int getIndex(){ return m_index; }
        QString getAlias() { return m_alias; }
        bool isEnabled() { return m_enabled; }
        qreal getRange() { return m_range; }
        qreal getOffset() { return m_offset; }
    protected:
        int m_index;
        QString m_alias;
        bool m_enabled;        
        qreal m_range; //< Channel range, e.g., when m_range = 5.0, scope range is +5 V to -5 V (i.e., the whole range is 10 V)
        qreal m_offset;
    };

    virtual ~TScope() {}

    /// Scope name
    virtual QString getName() const = 0;
    /// Scope info
    virtual QString getInfo() const = 0;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const = 0;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) = 0;

    /// Initialize the scope
    virtual void init(bool *ok = nullptr) = 0;
    /// Deinitialize the scope
    virtual void deInit(bool *ok = nullptr) = 0;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const = 0;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) = 0;

    /// Run the oscilloscope: wait for trigger when set, otherwise capture immediately
    virtual void run(size_t * expectedBufferSize, bool *ok = nullptr) = 0;
    /// Stop the oscilloscope
    virtual void stop(bool *ok = nullptr) = 0;

    /// Downloads samples from the oscilloscope, first waits for the aquisition to complete. Works with a char memory buffer, fills it with an array of samplesType values (typically will need a recast!). Returns size in bytes.
    virtual size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage) = 0;

    /// Get channel info
    virtual QList<TChannelStatus> getChannelsStatus() = 0;

};

#endif // TSCOPE_H
