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
// Petr Socha (initial author)

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

    /// Returns number of bytes available for reading, when supported
    virtual std::optional<size_t> availableBytes() = 0;

};

#endif // TIODEVICE_H
