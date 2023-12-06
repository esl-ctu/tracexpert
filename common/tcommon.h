#ifndef TCOMMON_H
#define TCOMMON_H

#include <QString>
#include "tconfigparam.h"

class TCommon {

public:
    virtual ~TCommon() {}

    /// Plugin name
    virtual QString getName() const = 0;
    /// Plugin info
    virtual QString getInfo() const = 0;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const = 0;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) = 0;

    /// Initialize the plugin
    virtual void init(bool *ok = nullptr) = 0;
    /// Deinitialize the plugin
    virtual void deInit(bool *ok = nullptr) = 0;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const = 0;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) = 0;

};

#endif // TCOMMON_H
