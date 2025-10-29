#ifndef TANALSTREAMRECEIVERMODEL_H
#define TANALSTREAMRECEIVERMODEL_H

#include "../../common/receiver/treceivermodel.h"

#include "tanalstreamreceiver.h"

class TAnalStreamReceiverModel : public TReceiverModel
{
public:
    explicit TAnalStreamReceiverModel(TAnalStreamReceiver * receiver, QObject * parent = nullptr);

    QString name();
    QString info();

private:
    TAnalStreamReceiver * m_analReceiver;
};

#endif // TANALSTREAMRECEIVERMODEL_H
