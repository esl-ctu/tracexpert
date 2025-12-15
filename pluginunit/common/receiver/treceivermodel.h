#ifndef TRECEIVERMODEL_H
#define TRECEIVERMODEL_H

#include <QThread>

#include "treceiver.h"

class TReceiverModel : public QObject
{
    Q_OBJECT

public:
    explicit TReceiverModel(TReceiver * receiver, QObject * parent = nullptr);
    ~TReceiverModel();

    QString name();
    QString info();

    bool isBusy();

    std::optional<size_t> availableBytes();

    QByteArray receivedData() const;

public slots:
    void readData(int length);

    void enableAutoRead();
    void disableAutoRead();

    void clearReceivedData();

signals:
    void dataRead(QByteArray data);
    void readFailed();
    void readBusy();

private slots:
    void dataReceived(QByteArray data);
    void receiveFailed();

private:
    TReceiver * m_receiver;
    QThread * m_receiverThread;
    bool m_receiving;
    bool m_autoReceive;

    QByteArray m_receivedData;

signals:
    void receiveData(int length);
    void startReceiving();
    void stopReceiving();
};

#endif // TRECEIVERMODEL_H
