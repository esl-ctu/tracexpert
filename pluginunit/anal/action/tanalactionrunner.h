#ifndef TANALACTIONRUNNER_H
#define TANALACTIONRUNNER_H

#include <QObject>

#include "tanaldevice.h"

class TAnalActionRunner : public QObject
{
    Q_OBJECT

public:
    explicit TAnalActionRunner(TAnalAction * action, QObject * parent = nullptr);

public slots:
    void run();

signals:
    void started();
    void finished();

private:
    TAnalAction * m_action;
};

#endif // TANALACTIONRUNNER_H
