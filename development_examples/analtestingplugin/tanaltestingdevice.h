// COPYRIGHT HEADER BEGIN
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
// COPYRIGHT HEADER END

#ifndef TANALTESTINGDEVICE_H
#define TANALTESTINGDEVICE_H

#include <QThread>
#include <QString>
#include <QRandomGenerator>
#include "tconfigparam.h"
#include "tanaldevice.h"


class TAnalTestingDevice : public TAnalDevice {

public:
    TAnalTestingDevice();
    virtual ~TAnalTestingDevice();

    /// AnalDevice name
    virtual QString getName() const override;
    /// AnalDevice info
    virtual QString getInfo() const override;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const override;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    /// Initialize the analytic device
    virtual void init(bool *ok = nullptr) override;
    /// Deinitialize the analytic device
    virtual void deInit(bool *ok = nullptr) override;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const override;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    /// Get list of available actions
    virtual QList<TAnalAction *> getActions() const override;

    /// Get list of available input data streams
    virtual QList<TAnalInputStream *> getInputDataStreams() const override;
    /// Get list of available output data streams
    virtual QList<TAnalOutputStream *> getOutputDataStreams() const override;

    virtual bool isBusy() const override;

    size_t fillData(const uint8_t * buffer, size_t length, QList<QList<uint8_t> *> & set);
    void processData(bool subtract);
    size_t getData(uint8_t * buffer, size_t length);

private:

    TConfigParam m_preInitParams;
    size_t m_traceLength;

    QList<TAnalAction *> m_analActions;

    QList<TAnalInputStream *> m_analInputStreams;
    QList<TAnalOutputStream *> m_analOutputStreams;

    QList<QList<uint8_t> *> m_firstSet;
    QList<QList<uint8_t> *> m_secondSet;

    size_t m_length;
    int m_position;
    int * m_data = nullptr;
};

#endif // TANALTESTINGDEVICE_H
