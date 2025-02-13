#ifndef TANALACTIONABORTER_H
#define TANALACTIONABORTER_H

#include <QObject>

#include "tanaldevice.h"

class TAnalActionAborter : public QObject
{
    Q_OBJECT

public:
    explicit TAnalActionAborter(TAnalAction * action, QObject *parent = nullptr);

public slots:
    void abort();

private:
    TAnalAction * m_action;
};

#endif // TANALACTIONABORTER_H
