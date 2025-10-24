#ifndef TANALDEVICEMODEL_H
#define TANALDEVICEMODEL_H

#include "../common/tdevicemodel.h"
#include "tanaldevice.h"
#include "stream/tanalstreamsendermodel.h"
#include "stream/tanalstreamreceivermodel.h"
#include "action/tanalactionmodel.h"

class TAnalDeviceContainer;

class TAnalDeviceModel : public TDeviceModel
{
    Q_OBJECT

public:
    explicit TAnalDeviceModel(TAnalDevice * IODevice, TAnalDeviceContainer * parent, bool manual = false);
    ~TAnalDeviceModel();

    void show() override;

    bool init() override;
    bool deInit() override;

    bool remove() override;

    virtual void bind(TCommon * unit) override;
    virtual void release() override;

    QList<TAnalStreamSenderModel *> senderModels();
    QList<TAnalStreamReceiverModel *> receiverModels();
    QList<TAnalActionModel *> actionModels();

signals:
    void initialized(TAnalDeviceModel * analDevice);
    void deinitialized(TAnalDeviceModel * analDevice);
    void showRequested(TAnalDeviceModel * analDevice);
    void removeRequested(TAnalDeviceModel * analDevice);

private:
    TAnalDevice * m_analDevice;
    QList<TAnalStreamSenderModel *> m_senderModels;
    QList<TAnalStreamReceiverModel *> m_receiverModels;
    QList<TAnalActionModel *> m_actionModels;
};

#endif // TANALDEVICEMODEL_H
