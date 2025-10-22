#ifndef TANALDEVICEWIDGET_H
#define TANALDEVICEWIDGET_H

#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QPlainTextEdit>

#include "protocol/tprotocolcontainer.h"
#include "tanaldevicemodel.h"
#include "tconfigparamwidget.h"
#include "tmessageformmanager.h"

class TAnalDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TAnalDeviceWidget(TAnalDeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    ~TAnalDeviceWidget();

public slots:
    bool applyPostInitParam();

    void setAutoreceive(bool enabled);
    void receiveBytes();
    void receiveBusy();
    void receiveFailed();
    void dataReceived(QByteArray data, TAnalStreamReceiverModel * receiverModel);

    void sendFile(QString fileName);
    void receiveFile(QString fileName);

    void sendBytes();
    void sendRawBytes();
    void sendProtocolBytes();
    void sendBusy();
    void sendFailed();
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
    QString byteArraytoHumanReadableString(const QByteArray & byteArray);

    TAnalDeviceModel * m_deviceModel;

    QList<TAnalStreamSenderModel *> m_senderModels;
    QList<TAnalStreamReceiverModel *> m_receiverModels;
    QList<TAnalActionModel *> m_actionModels;

    TAnalStreamSenderModel * m_currentSenderModel;
    TAnalStreamReceiverModel * m_currentReceiverModel;
    TAnalActionModel * m_currentActionModel;

    TProtocolContainer * m_protocolContainer;

    TConfigParamWidget * m_paramWidget;

    TProtocol m_selectedProtocol;
    TMessage m_selectedMessage;

    TMessageFormManager * m_messageFormManager;

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

    QList<QByteArray *> m_receivedData;
};

#endif // TANALDEVICEWIDGET_H
