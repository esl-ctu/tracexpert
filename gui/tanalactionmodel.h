#ifndef TANALACTIONMODEL_H
#define TANALACTIONMODEL_H

#include <QThread>

#include "tanaldevice.h"
#include "tanalactionrunner.h"
#include "tanalactionaborter.h"

class TAnalActionModel : public QObject
{
    Q_OBJECT

public:
    explicit TAnalActionModel(TAnalAction * action, QObject * parent = nullptr);
    ~TAnalActionModel();

    QString name();
    QString info();

    bool isEnabled() const;

private:
    TAnalAction * m_action;
    TAnalActionRunner * m_runner;
    TAnalActionAborter * m_aborter;

    QThread * m_runnerThread;
    QThread * m_aborterThread;

signals:
    void run();
    void abort();
    void started();
    void finished();
};

#endif // TANALACTIONMODEL_H
