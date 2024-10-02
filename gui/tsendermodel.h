#ifndef TSENDERMODEL_H
#define TSENDERMODEL_H

#include <QThread>

#include "tsender.h"

class TSenderModel : public QObject
{
    Q_OBJECT
public:
    explicit TSenderModel(TSender * sender, QObject * parent = nullptr);
    ~TSenderModel();

public slots:
    void writeData(QByteArray data);

signals:
    void dataWritten(QByteArray data);
    void writeFailed();
    void writeBusy();

private slots:
    void dataSent(QByteArray data);
    void sendFailed();

private:
    TSender * m_sender;
    QThread * m_senderThread;
    bool m_sending;

signals:
    void sendData(QByteArray data);
};

#endif // TSENDERMODEL_H
