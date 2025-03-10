#ifndef TCPAACTION_H
#define TCPAACTION_H

#include "tcpadevice.h"

class TCPAAction : public TAnalAction
{
public:
    explicit TCPAAction(QString name, QString info, std::function<void(void)> run);

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
};

#endif // TCPAACTION_H

