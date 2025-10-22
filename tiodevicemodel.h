#ifndef TIODEVICEMODEL_H
#define TIODEVICEMODEL_H

#include <QThread>

#include "tdevicemodel.h"
#include "tiodevice.h"
#include "tsendermodel.h"
#include "treceivermodel.h"

class TIODeviceContainer;

class TIODeviceModel : public TDeviceModel
{
    Q_OBJECT

public:
    explicit TIODeviceModel(TIODevice * IODevice, TIODeviceContainer * parent, bool manual = false);
    ~TIODeviceModel();

    void show() override;

    bool init() override;
    bool deInit() override;

    bool remove() override;

    virtual void bind(TCommon * unit) override;
    virtual void release() override;

    TSenderModel * senderModel();
    TReceiverModel * receiverModel();

signals:
    void initialized(TIODeviceModel * IODevice);
    void deinitialized(TIODeviceModel * IODevice);
    void showRequested(TIODeviceModel * IODevice);
    void removeRequested(TIODeviceModel * IODevice);

private:
    TIODevice * m_IODevice;
    TSenderModel * m_senderModel = nullptr;
    TReceiverModel * m_receiverModel = nullptr;
};

#endif // TIODEVICEMODEL_H
