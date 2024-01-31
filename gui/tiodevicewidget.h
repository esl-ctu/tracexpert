#ifndef TIODEVICEWIDGET_H
#define TIODEVICEWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPlainTextEdit>

#include "protocol/tprotocolcontainer.h"
#include "qboxlayout.h"
#include "tiodevicemodel.h"
#include "tconfigparamwidget.h"
#include "tmessageformwidget.h"

class TIODeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TIODeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);

public slots:
    bool applyPostInitParam();

    void setAutoreceive(bool enabled);
    void receiveBytes();
    void receiveBusy();
    void receiveFailed();
    void dataReceived(QByteArray data);

    void sendRawBytes();
    void sendProtocolBytes();
    void sendBusy();
    void sendFailed();
    //void selectSendMessageValidator();

private:
    void sendProtocolChanged(int index);
    void sendMessageChanged(int index);

    void updateDisplayedProtocols();

    TIODeviceModel * m_deviceModel;
    TProtocolContainer * m_protocolContainer;

    TConfigParamWidget * m_paramWidget;

    TProtocol m_selectedProtocol;
    TMessage m_selectedMessage;

    TMessageFormWidget * m_messageFormWidget;
    QLineEdit * m_receiveBytesEdit;
    QComboBox * m_receiveProtocolComboBox;

    QComboBox * m_sendProtocolComboBox;
    QComboBox * m_sendMessageComboBox;
    QVBoxLayout * m_sendMessageLayout;
    QWidget * m_sendRawMessageWidget;
    QWidget * m_sendProtocolMessageWidget;
    QLineEdit * m_sendMessageEdit;

    QPlainTextEdit * m_communicationLogTextEdit;
};

#endif // TIODEVICEWIDGET_H
