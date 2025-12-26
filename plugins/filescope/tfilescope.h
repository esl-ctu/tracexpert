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
// Vojtěch Miškovský (initial author)

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
