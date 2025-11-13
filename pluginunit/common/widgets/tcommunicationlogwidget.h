#ifndef TCOMMUNICATIONLOGWIDGET_H
#define TCOMMUNICATIONLOGWIDGET_H

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

    explicit TCommunicationLogWidget(QList<TSenderModel *> senderModels, QList<TReceiverModel *> receiverModels, QWidget * parent = nullptr);

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

    void messageSent(TMessage message, TSenderModel * sender);
    void protocolChanged(TProtocol protocol, TReceiverModel * receiver);

private:
    QString byteArraytoHumanReadableString(const QByteArray & byteArray);

    QPlainTextEdit * m_textEdit;
    QComboBox * m_format;

    QList<TSenderModel *> m_senderModels;
    QList<TReceiverModel *> m_receiverModels;

    QMap<TSenderModel *, TMessage> m_sentMessages;
    QMap<TReceiverModel *, TProtocol> m_protocols;
};

#endif // TCOMMUNICATIONLOGWIDGET_H
