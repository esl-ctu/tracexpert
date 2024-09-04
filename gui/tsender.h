#ifndef TSENDER_H
#define TSENDER_H

#include <QObject>

class TSender : public QObject
{
    Q_OBJECT

public:
    explicit TSender(QObject * parent = nullptr);

public slots:
    void sendData(QByteArray data);

protected:
    virtual size_t writeData(const uint8_t * buffer, size_t len) = 0;

signals:
    void dataSent(QByteArray data);
    void sendFailed();
};

#endif // TSENDER_H
