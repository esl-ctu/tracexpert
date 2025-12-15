#ifndef TANALDEVICE_H
#define TANALDEVICE_H

#include <QString>
#include "tconfigparam.h"
#include "tcommon.h"

class TAnalStream {
public:

    virtual ~TAnalStream() {}

    /// AnalStream name
    virtual QString getName() const = 0;
    /// AnalStream info
    virtual QString getInfo() const = 0;
};

class TAnalInputStream : public TAnalStream {
public:

    /// Receives the specified amount of data into the buffer
    virtual size_t readData(uint8_t * buffer, size_t len) = 0;
    virtual std::optional<size_t> availableBytes() = 0;
};

class TAnalOutputStream : public TAnalStream {
public:

    /// Receives the specified amount of data into the buffer
    virtual size_t writeData(const uint8_t * buffer, size_t len) = 0;
};

class TAnalAction {

public:

    virtual ~TAnalAction() {}

    /// AnalAction name
    virtual QString getName() const = 0;
    /// AnalAction info
    virtual QString getInfo() const = 0;

    /// Get info on analytic action's availability
    virtual bool isEnabled() const = 0;
    /// Run the analytic action
    virtual void run() = 0;
    /// Abort the current run of analytic action
    virtual void abort() = 0;
};

class TAnalDevice : public TCommon {

public:

    virtual ~TAnalDevice() {}

    /// AnalDevice name
    virtual QString getName() const = 0;
    /// AnalDevice info
    virtual QString getInfo() const = 0;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const = 0;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) = 0;

    /// Initialize the analytic device
    virtual void init(bool *ok = nullptr) = 0;
    /// Deinitialize the analytic device
    virtual void deInit(bool *ok = nullptr) = 0;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const = 0;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) = 0;

    /// Get list of available actions
    virtual QList<TAnalAction *> getActions() const = 0;

    /// Get list of available input data streams
    virtual QList<TAnalInputStream *> getInputDataStreams() const = 0;
    /// Get list of available output data streams
    virtual QList<TAnalOutputStream *> getOutputDataStreams() const = 0;

    virtual bool isBusy() const = 0;
};

#endif // TANALDEVICE_H
