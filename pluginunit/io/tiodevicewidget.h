#ifndef TIODEVICEWIDGET_H
#define TIODEVICEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>

#include "../../protocol/tprotocolcontainer.h"
#include "tiodevicemodel.h"
#include "widgets/tconfigparamwidget.h"
#include "../../tmessageformmanager.h"

class TIODeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TIODeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    ~TIODeviceWidget();

public slots:
    bool applyPostInitParam();

    void setAutoreceive(bool enabled);
    void receiveBytes();
    void receiveBusy();
    void receiveFailed();
    void dataReceived(QByteArray data);

    void sendBytes();
    void sendRawBytes();
    void sendProtocolBytes();

    void sendFile(QString fileName);
    void receiveFile(QString fileName);

    void sendBusy();
    void sendFailed();
    void dataSent(QByteArray data);
    //void selectSendMessageValidator();

    bool validateRawInputValues();

private slots:
    void sendProtocolChanged(int index);
    void sendMessageChanged(int index);

    void updateDisplayedProtocols();

private:
    QString byteArraytoHumanReadableString(const QByteArray & byteArray);

    TIODeviceModel * m_deviceModel;
    TSenderModel * m_senderModel;
    TReceiverModel * m_receiverModel;
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

    QByteArray m_receivedData;
};

#endif // TIODEVICEWIDGET_H
