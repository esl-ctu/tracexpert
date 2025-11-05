#ifndef TCOMMUNICATIONDEVICEWIDGET_H
#define TCOMMUNICATIONDEVICEWIDGET_H

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>

#include "../../protocol/tprotocolcontainer.h"
#include "../../anal/tanaldevicemodel.h"
#include "../../io/tiodevicemodel.h"
#include "../../../common/widgets/tconfigparamwidget.h"
#include "../tmessageformmanager.h"

#define DISPLAY_DATA_LENGTH_LIMIT 300

class TCommunicationDeviceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TCommunicationDeviceWidget(TAnalDeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    explicit TCommunicationDeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    ~TCommunicationDeviceWidget();

public slots:
    bool applyPostInitParam();

    void setAutoreceive(bool enabled);
    void receiveBytes();
    void receiveBusy();
    void receiveFailed();
    void dataReceived(QByteArray data, TReceiverModel * receiverModel);

    void sendBytes();
    void sendRawBytes();
    void sendProtocolBytes();

    void sendFile(QString fileName);
    void receiveFile(QString fileName);

    void sendBusy();
    void sendFailed();
    void dataSent(QByteArray data, TSenderModel * senderModel);
    //void selectSendMessageValidator();

    bool validateRawInputValues();

private slots:
    void sendProtocolChanged(int index);
    void sendMessageChanged(int index);
    void senderChanged(int index);
    void receiverChanged(int index);
    void actionChanged(int index);

    void runAction();
    void abortAction();

    void updateDisplayedProtocols();

private:
    void init();

    QString byteArraytoHumanReadableString(const QByteArray & byteArray);

    TDeviceModel * m_deviceModel;

    QList<TSenderModel *> m_senderModels;
    QList<TReceiverModel *> m_receiverModels;
    QList<TAnalActionModel *> m_actionModels;

    TSenderModel * m_currentSenderModel;
    TReceiverModel * m_currentReceiverModel;
    TAnalActionModel * m_currentActionModel;

    TProtocolContainer * m_protocolContainer;

    TConfigParamWidget * m_paramWidget;

    TProtocol m_selectedProtocol;
    TMessage m_selectedMessage;

    TMessageFormManager * m_messageFormManager;
    TMessage m_messageToBeSent;

    QLineEdit * m_receiveBytesEdit;
    QComboBox * m_receiveProtocolComboBox;

    QComboBox * m_sendProtocolComboBox;
    QComboBox * m_sendMessageComboBox;
    QLabel * m_noMessagesLabel;
    QPushButton * m_sendButton;
    QFormLayout * m_sendFormLayout;

    QLineEdit * m_rawMessageEdit;
    QComboBox * m_rawFormatComboBox;
    QHBoxLayout * m_rawMessageEditLayout;

    QPlainTextEdit * m_communicationLogTextEdit;
    QComboBox * m_logFormat;

    QList<QByteArray *> m_receivedData;
};

#endif // TCOMMUNICATIONDEVICEWIDGET_H
