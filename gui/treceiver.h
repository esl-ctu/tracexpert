#ifndef TRECEIVER_H
#define TRECEIVER_H

#include <QObject>

#define DATA_BLOCK_SIZE 64
#define AUTORECEIVE_DELAY_MS 20

class TReceiver : public QObject
{
    Q_OBJECT

public:
    explicit TReceiver(QObject * parent = nullptr);

public slots:
    void receiveData(int length);
    void startReceiving();
    void stopReceiving();

protected:
    virtual size_t readData(uint8_t * buffer, size_t len) = 0;

private:
    bool m_stopReceiving;

signals:
    void dataReceived(QByteArray data);
    void receiveFailed();
};

#endif // TRECEIVER_H
