#ifndef TSENDERWIDGET_H
#define TSENDERWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>

#include "../../protocol/tprotocolcontainer.h"
#include "tmessageformmanager.h"
#include "../sender/tsendermodel.h"

class TSenderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TSenderWidget(TSenderModel * senderModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    ~TSenderWidget();

signals:
    void messageSent(TMessage message, TSenderModel * sender);

public slots:
    void sendBytes();
    void sendRawBytes();
    void sendProtocolBytes();

    void sendFile(QString fileName);

    bool validateRawInputValues();

private slots:
    void protocolChanged(int index);
    void messageChanged(int index);

    void updateDisplayedProtocols();

private:
    TSenderModel * m_senderModel;

    QComboBox * m_protocolComboBox;
    QComboBox * m_messageComboBox;
    QLabel * m_noMessagesLabel;
    QPushButton * m_sendButton;
    QFormLayout * m_formLayout;

    TProtocolContainer * m_protocolContainer;

    TMessageFormManager * m_messageFormManager;

    TProtocol m_selectedProtocol;
    TMessage m_selectedMessage;

    QLineEdit * m_rawMessageEdit;
    QComboBox * m_rawFormatComboBox;
    QHBoxLayout * m_rawMessageEditLayout;
};

#endif // TSENDERWIDGET_H
