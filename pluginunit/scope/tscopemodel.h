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
// Adam Švehla
// Petr Socha

#ifndef TSCOPEMODEL_H
#define TSCOPEMODEL_H

#include <QObject>
#include <QThread>

#include "../common/tdevicemodel.h"
#include "tscope.h"

class TScopeCollector : public QObject
{
    Q_OBJECT

public:
    explicit TScopeCollector(TScope * scope, QObject * parent = nullptr);

public slots:
    void collectData(size_t bufferSize);

signals:
    void dataCollected(size_t traces, size_t samples, TScope::TSampleType type, QList<QByteArray> buffers, bool overvoltage);
    void collectionStopped();
    void nothingCollected();

private:
    TScope * m_scope;
};

class TScopeContainer;

class TScopeModel : public TDeviceModel
{
    Q_OBJECT

public:
    explicit TScopeModel(TScope * scope, TScopeContainer * parent, bool manual = false);
    ~TScopeModel();

    void show() override;

    bool init() override;
    bool deInit() override;

    bool remove() override;

    virtual TConfigParam setPostInitParams(const TConfigParam & param) override;

    virtual void bind(TCommon * unit) override;
    virtual void release() override;

    QList<TScope::TChannelStatus> channelsStatus();
    TScope::TTriggerStatus triggerStatus();
    TScope::TTimingStatus timingStatus();

signals:
    void initialized(TScopeModel * scope);
    void deinitialized(TScopeModel * scope);
    void showRequested(TScopeModel * scope);
    void removeRequested(TScopeModel * scope);

    void channelsStatusChanged();

    // External signals
    void runFailed();
    void stopFailed();
    void downloadFailed();

    void tracesDownloaded(size_t traces, size_t samples, TScope::TSampleType type, QList<QByteArray> buffers, bool overvoltage);
    void tracesEmpty();
    void stopped();

    // Internal signals
    void startDataCollection(size_t bufferSize);

public slots:
    void run();
    void runSingle();
    void stop();

private slots:
    void dataCollected(size_t traces, size_t samples, TScope::TSampleType type, QList<QByteArray> buffers, bool overvoltage);

    void dataCollectionStopped();
    void noDataCollected();

private:
    void run(bool repeat);

    TScope * m_scope;

    bool m_repeat;
    bool m_stopping;

    friend class TScopeCollector;

    TScopeCollector * m_collector;

    QThread collectorThread;
};

#endif // TSCOPEMODEL_H
