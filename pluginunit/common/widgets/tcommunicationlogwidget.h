#ifndef TCOMMUNICATIONLOGWIDGET_H
#define TCOMMUNICATIONLOGWIDGET_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QComboBox>

#include "../receiver/treceivermodel.h"
#include "../sender/tsendermodel.h"
#include "tprotocol.h"

#define DISPLAY_DATA_LENGTH_LIMIT 300

class TCommunicationLogWidget : public QWidget
{
    Q_OBJECT

public:    
    enum Direction { Sent, Received };

    explicit TCommunicationLogWidget(QList<TSenderModel *> senderModels, QList<TReceiverModel *> receiverModels, TProtocol & protocol, TMessage & messageToBeSent, QWidget * parent = nullptr);

    void append(QString header, QString message = QString(), QColor color = QColorConstants::Black);
    void appendCommunication(Direction direction, QString data, qsizetype size = -1, QString origin = QString());

    QByteArray receivedData(TReceiverModel * receiverModel);

public slots:
    void receiveBusy();
    void receiveFailed();
    void dataReceived(QByteArray data, TReceiverModel * receiverModel);

    void sendBusy();
    void sendFailed();
    void dataSent(QByteArray data, TSenderModel * senderModel);

private:
    QString byteArraytoHumanReadableString(const QByteArray & byteArray);

    QPlainTextEdit * m_textEdit;
    QComboBox * m_format;

    TProtocol & m_protocol;
    TMessage & m_messageToBeSent;

    QList<TSenderModel *> m_senderModels;
    QList<TReceiverModel *> m_receiverModels;

    QMap<TReceiverModel *, QByteArray> m_receivedData;
};

#endif // TCOMMUNICATIONLOGWIDGET_H
