#ifndef TIODEVICEMODEL_H
#define TIODEVICEMODEL_H

#include <QThread>

#include "tpluginunitmodel.h"
#include "tiodevice.h"

#define DATA_BLOCK_SIZE 64
#define AUTORECEIVE_DELAY_MS 20

class TIODeviceReceiver : public QObject
{
    Q_OBJECT

public:
    explicit TIODeviceReceiver(TIODevice * IODevice, QObject * parent = nullptr);

public slots:
    void receiveData(int length);
    void startReceiving();
    void stopReceiving();

private:
    TIODevice * m_IODevice;
    bool m_stopReceiving;

signals:
    void dataReceived(QByteArray data);
    void receiveFailed();
};

class TIODeviceSender : public QObject
{
    Q_OBJECT

public:
    explicit TIODeviceSender(TIODevice * IODevice, QObject * parent = nullptr);

public slots:
    void sendData(QByteArray data);

private:
    TIODevice * m_IODevice;

signals:
    void dataSent(QByteArray data);
    void sendFailed();
};

class TIODeviceContainer;

class TIODeviceModel : public TPluginUnitModel
{
    Q_OBJECT

public:
    explicit TIODeviceModel(TIODevice * IODevice, TIODeviceContainer * parent);
    ~TIODeviceModel();

    void show();

    bool init() override;
    bool deInit() override;

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QVariant status() const override;

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
