#ifndef TIODEVICE_H
#define TIODEVICE_H

#include <QString>
#include "tconfigparam.h"
#include "tcommon.h"

class TIODevice : public TCommon {

public:

    virtual ~TIODevice() {}

    /// IODevice name
    virtual QString getName() const = 0;
    /// IODevice info
    virtual QString getInfo() const = 0;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const = 0;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) = 0;

    /// Initialize the IO device
    virtual void init(bool *ok = nullptr) = 0;
    /// Deinitialize the IO device
    virtual void deInit(bool *ok = nullptr) = 0;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const = 0;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) = 0;

    /// Sends out the specified amount of data from buffer
    virtual size_t writeData(const uint8_t * buffer, size_t len) = 0;
    /// Receives the specified amount of data into the buffer
    virtual size_t readData(uint8_t * buffer, size_t len) = 0;

};

#endif // TIODEVICE_H
