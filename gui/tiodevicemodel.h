#ifndef TIODEVICEMODEL_H
#define TIODEVICEMODEL_H

#include <QThread>

#include "tdevicemodel.h"
#include "tiodevice.h"
#include "treceiver.h"
#include "tsender.h"

class TIODeviceReceiver : public TReceiver
{
    Q_OBJECT

public:
    explicit TIODeviceReceiver(TIODevice * IODevice, QObject * parent = nullptr);

protected:
    size_t readData(uint8_t * buffer, size_t len) override;

private:
    TIODevice * m_IODevice;
};

class TIODeviceSender : public TSender
{
    Q_OBJECT

public:
    explicit TIODeviceSender(TIODevice * IODevice, QObject * parent = nullptr);

protected:
    size_t writeData(const uint8_t * buffer, size_t len) override;

private:
    TIODevice * m_IODevice;
};

class TIODeviceContainer;

class TIODeviceModel : public TDeviceModel
{
    Q_OBJECT

public:
    explicit TIODeviceModel(TIODevice * IODevice, TIODeviceContainer * parent, bool manual = false);

    void show();

    bool init() override;
    bool deInit() override;

    bool remove() override;

    virtual void bind(TCommon * unit) override;
    virtual void release() override;

signals:
    void initialized(TIODeviceModel * IODevice);
    void deinitialized(TIODeviceModel * IODevice);
    void showRequested(TIODeviceModel * IODevice);
    void removeRequested(TIODeviceModel * IODevice);

public slots:
    void writeData(QByteArray data);
    void readData(int length);

    void enableAutoRead();
    void disableAutoRead();

signals:
    void dataWritten(QByteArray data);
    void dataRead(QByteArray data);

    void writeFailed();
    void readFailed();

    void writeBusy();
    void readBusy();

private slots:
    void dataSent(QByteArray data);
    void dataReceived(QByteArray data);

    void sendFailed();
    void receiveFailed();

private:
    TIODevice * m_IODevice;

    friend class TIODeviceReceiver;
    friend class TIODeviceSender;

    TIODeviceReceiver * m_receiver;
    TIODeviceSender * m_sender;

    QThread senderThread;
    QThread receiverThread;

    bool m_receiving;
    bool m_sending;
    bool m_autoReceive;

signals:
    void sendData(QByteArray data);
    void receiveData(int length);

    void startReceiving();
    void stopReceiving();
};

#endif // TIODEVICEMODEL_H
