#ifndef TSCOPE_H
#define TSCOPE_H

#include <QString>
#include "tconfigparam.h"

class TScope {

public:

    virtual ~TScope() {}

    /// IODevice name
    virtual QString getIODeviceName() const = 0;
    /// IODevice info
    virtual QString getIODeviceInfo() const = 0;

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

    /// Run the oscilloscope: wait for trigger when triggered, otherwise capture immediately
    virtual void run() = 0;
    /// Stop the oscilloscope
    virtual void stop() = 0;

    /// Downloads values from the oscilloscope, first waits for the aquisition to complete
    virtual size_t getValues(int channel, int16_t * buffer, size_t len) = 0; // CHAR8 TODO

};

#endif // TSCOPE_H
