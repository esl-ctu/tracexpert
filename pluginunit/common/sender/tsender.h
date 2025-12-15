#ifndef TSENDER_H
#define TSENDER_H

#include <QObject>

class TSender : public QObject
{
    Q_OBJECT

public:
    explicit TSender(QObject * parent = nullptr);

    virtual QString name();
    virtual QString info();

    bool isBusy();

public slots:
    void sendData(QByteArray data);

protected:
    virtual size_t writeData(const uint8_t * buffer, size_t len) = 0;

signals:
    void dataSent(QByteArray data);
    void sendFailed();

private:
    bool m_isBusy = false;
};

#endif // TSENDER_H
