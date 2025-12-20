#ifndef TAIAPICONNECTENGINEDEVICEACTION_H
#define TAIAPICONNECTENGINEDEVICEACTION_H
#pragma once

#include "taiapiconnectengine.h"

class TAIAPIConnectEngineDeviceAction : public TAnalAction {
public:
    explicit TAIAPIConnectEngineDeviceAction(QString name, QString info, std::function<void(void)> run);

    /// AnalAction name
    virtual QString getName() const override;
    /// AnalAction info
    virtual QString getInfo() const override;

    /// Get info on analytic action's availability
    virtual bool isEnabled() const override;
    /// Run the analytic action
    virtual void run() override;
    /// Abort the current run of analytic action
    virtual void abort() override;

private:
    QString m_name;
    QString m_info;
    std::function<void(void)> m_run;
    bool * m_deviceInitialized;

};

#endif // TAIAPICONNECTENGINEDEVICEACTION_H
