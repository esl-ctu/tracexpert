#include "tiodevicewidget.h"

#include <QLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QCoreApplication>

#include "qcheckbox.h"
#include "qcombobox.h"
#include "qformlayout.h"
#include "tconfigparamwidget.h"
#include "tdialog.h"

TIODeviceWidget::TIODeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel), m_protocolContainer(protocolContainer)
{
    setWindowTitle(tr("IO Device - %1").arg(m_deviceModel->name()));

    connect(m_deviceModel, &TIODeviceModel::dataRead, this, &TIODeviceWidget::dataReceived);

    m_communicationLogTextEdit = new QPlainTextEdit;
    m_communicationLogTextEdit->setReadOnly(true);
    
    m_paramWidget = new TConfigParamWidget(m_deviceModel->postInitParams());

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TIODeviceWidget::applyPostInitParam);

    QVBoxLayout * paramLayout = new QVBoxLayout;
    paramLayout->addWidget(m_paramWidget);
    paramLayout->addWidget(applyButton);

    QHBoxLayout * textParamLayout = new QHBoxLayout;

    textParamLayout->addWidget(m_communicationLogTextEdit);
    textParamLayout->addLayout(paramLayout);

    QGroupBox * textParamBox = new QGroupBox;
    textParamBox->setLayout(textParamLayout);

    m_sendMessageEdit = new QLineEdit;

    QRadioButton * hexRadioButton = new QRadioButton("Hex");
    hexRadioButton->setChecked(true);
    QRadioButton * asciiRadioButton = new QRadioButton("ASCII");

    QLayout * radioLayout = new QVBoxLayout;
    radioLayout->addWidget(hexRadioButton);
    radioLayout->addWidget(asciiRadioButton);

    QGroupBox * radioGroupBox = new QGroupBox("Format");
    radioGroupBox->setLayout(radioLayout);

    QPushButton * sendRawButton = new QPushButton("Send");
    connect(sendRawButton, &QPushButton::clicked, this, &TIODeviceWidget::sendRawBytes);

    QHBoxLayout * sendRawMessageLayout = new QHBoxLayout;
    sendRawMessageLayout->addWidget(m_sendMessageEdit);
    sendRawMessageLayout->addWidget(radioGroupBox);
    sendRawMessageLayout->addWidget(sendRawButton);
    sendRawMessageLayout->setContentsMargins(0, 0, 0, 0);

    m_sendRawMessageWidget = new QWidget;
    m_sendRawMessageWidget->setLayout(sendRawMessageLayout);

    QPushButton * sendProtocolButton = new QPushButton("Send");
    connect(sendProtocolButton, &QPushButton::clicked, this, &TIODeviceWidget::sendProtocolBytes);

    m_sendMessageComboBox = new QComboBox;
    connect(m_sendMessageComboBox, &QComboBox::currentIndexChanged, this, &TIODeviceWidget::sendMessageChanged);

    QHBoxLayout * sendMessageComboBoxLayout = new QHBoxLayout;
    sendMessageComboBoxLayout->addWidget(new QLabel("Message"));
    sendMessageComboBoxLayout->addWidget(m_sendMessageComboBox);

    QFrame * separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFrameShadow(QFrame::Sunken);

    m_messageFormWidget = new TMessageFormWidget;

    QVBoxLayout * sendProtocolMessageLayout = new QVBoxLayout;
    sendProtocolMessageLayout->addLayout(sendMessageComboBoxLayout);
    sendProtocolMessageLayout->addWidget(separatorLine);
    sendProtocolMessageLayout->addWidget(m_messageFormWidget);
    sendProtocolMessageLayout->addWidget(sendProtocolButton);
    sendProtocolMessageLayout->setContentsMargins(0, 0, 0, 0);

    m_sendProtocolMessageWidget = new QWidget;
    m_sendProtocolMessageWidget->setLayout(sendProtocolMessageLayout);
    m_sendProtocolMessageWidget->hide();

    m_sendProtocolComboBox = new QComboBox;

    connect(m_sendProtocolComboBox, &QComboBox::currentIndexChanged, this, &TIODeviceWidget::sendProtocolChanged);

    QHBoxLayout * sendProtocolSelectionLayout = new QHBoxLayout;
    sendProtocolSelectionLayout->addWidget(new QLabel("Protocol"));
    sendProtocolSelectionLayout->addWidget(m_sendProtocolComboBox);

    m_sendMessageLayout = new QVBoxLayout;
    m_sendMessageLayout->addLayout(sendProtocolSelectionLayout);
    m_sendMessageLayout->addWidget(m_sendRawMessageWidget);
    m_sendMessageLayout->addWidget(m_sendProtocolMessageWidget);

    QGroupBox * sendMessageBox = new QGroupBox("Send data");
    sendMessageBox->setLayout(m_sendMessageLayout);

    m_receiveBytesEdit = new QLineEdit;
    QIntValidator * receiveBytesValidator = new QIntValidator;
    receiveBytesValidator->setBottom(1);
    m_receiveBytesEdit->setValidator(receiveBytesValidator);

    QPushButton * receiveButton = new QPushButton("Receive");
    receiveButton->setEnabled(false);
    connect(receiveButton, &QPushButton::clicked, this, &TIODeviceWidget::receiveBytes);

    connect(m_receiveBytesEdit, &QLineEdit::textChanged, this, [=](){receiveButton->setEnabled(m_receiveBytesEdit->hasAcceptableInput());});

    connect(m_deviceModel, &TIODeviceModel::readBusy, this, &TIODeviceWidget::receiveBusy);

    QCheckBox * autoReceiveCheckbox = new QCheckBox;
    autoReceiveCheckbox->setChecked(false);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, this, &TIODeviceWidget::setAutoreceive);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, m_receiveBytesEdit, &QLineEdit::setDisabled);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, receiveButton, &QPushButton::setDisabled);

    m_receiveProtocolComboBox = new QComboBox;

    QFormLayout * receiveMessageLayout = new QFormLayout;
    receiveMessageLayout->addRow(tr("Autoreceive"), autoReceiveCheckbox);
    receiveMessageLayout->addRow(tr("Protocol"), m_receiveProtocolComboBox);
    receiveMessageLayout->addRow(tr("Bytes"), m_receiveBytesEdit);
    receiveMessageLayout->addWidget(receiveButton);

    QGroupBox * receiveMessageBox = new QGroupBox("Receive data");
    receiveMessageBox->setLayout(receiveMessageLayout);

    QHBoxLayout * sendReceiveLayout = new QHBoxLayout;
    sendReceiveLayout->addWidget(sendMessageBox);
    sendReceiveLayout->addWidget(receiveMessageBox);
    sendReceiveLayout->setStretch(0,1);
    sendReceiveLayout->setStretch(1,1);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(textParamBox);
    layout->addLayout(sendReceiveLayout);

    // temporary solution, will be replcaed by container signal
    QPushButton * updateProtocolsButton = new QPushButton("Update protocols (temporary button)");
    connect(updateProtocolsButton, &QPushButton::clicked, this, &TIODeviceWidget::updateDisplayedProtocols);
    layout->addWidget(updateProtocolsButton);

    updateDisplayedProtocols();

    setLayout(layout);

    connect(m_deviceModel, &TIODeviceModel::readFailed, this, &TIODeviceWidget::receiveFailed);
}

void TIODeviceWidget::updateDisplayedProtocols() {
    m_sendProtocolComboBox->clear();
    m_receiveProtocolComboBox->clear();

    m_sendProtocolComboBox->addItem("raw data");
    m_receiveProtocolComboBox->addItem("raw data");

    for(const TProtocolModel * protocolModel : m_protocolContainer->getItems()) {
        m_sendProtocolComboBox->addItem(protocolModel->protocol().getName());
        m_receiveProtocolComboBox->addItem(protocolModel->protocol().getName());
    }
}

void TIODeviceWidget::sendProtocolChanged(int index)
{
    if(index < 1) {
        m_sendProtocolMessageWidget->hide();
        m_sendRawMessageWidget->show();
        return;
    }

    m_sendRawMessageWidget->hide();
    m_sendProtocolMessageWidget->show();

    m_messageFormWidget->resetLayout();

    bool protocolFound;
    m_selectedProtocol = m_protocolContainer->getProtocolByName(m_sendProtocolComboBox->currentText(), &protocolFound);

    if(!protocolFound) {
        qWarning("Unknown protocol selected, maybe the user removed it?");
        return;
    }

    m_sendMessageComboBox->clear();
    for(const TMessage & message : m_selectedProtocol.getMessages()) {
        if(message.isResponse()) {
            continue;
        }

        m_sendMessageComboBox->addItem(message.getName());
    }
}

void TIODeviceWidget::sendMessageChanged(int index)
{
    if(index < 0)
        return;

    if(m_selectedProtocol.getName().isEmpty()) {
        qWarning("No protocol selected, cannot find message!");
        return;
    }

    bool messageFound;
    m_selectedMessage = m_selectedProtocol.getMessageByName(m_sendMessageComboBox->currentText(), &messageFound);

    if(!messageFound) {
        qWarning("Unknown message selected, maybe the user removed it?");
        m_messageFormWidget->resetLayout();
        return;
    }

    m_messageFormWidget->setMessage(m_selectedMessage);
}

bool TIODeviceWidget::applyPostInitParam()
{
    TConfigParam param = m_deviceModel->setPostInitParams(m_paramWidget->param());
    if (param.getState(true) == TConfigParam::TState::TError) {
        return false;
    };
    m_paramWidget->setParam(param);

    return true;
}

void TIODeviceWidget::setAutoreceive(bool enabled)
{
    if (enabled) {
        m_deviceModel->enableAutoRead();
    }
    else {
        m_deviceModel->disableAutoRead();
    }
}

void TIODeviceWidget::receiveBytes()
{
    m_deviceModel->readData(m_receiveBytesEdit->text().toInt());
}

void TIODeviceWidget::receiveBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}

void TIODeviceWidget::receiveFailed() {
    TDialog::deviceReceiveFailedMessage(this);
}

void TIODeviceWidget::dataReceived(QByteArray data)
{
    m_communicationLogTextEdit->appendHtml(QStringLiteral("<b>Received:</b>"));

    QString selectedProtocolName = m_receiveProtocolComboBox->currentText();

    if(selectedProtocolName == "raw data") {
        m_communicationLogTextEdit->appendPlainText(data);
        return;
    }

    bool protocolFound;
    TProtocol selectedReceiveProtocol = m_protocolContainer->getProtocolByName(selectedProtocolName, &protocolFound);

    if(!protocolFound) {
        qWarning("Unknown protocol selected, could not interpret message.");
        m_communicationLogTextEdit->appendPlainText(data);
        return;
    }

    TMessage matchedMessage = selectedReceiveProtocol.tryMatchResponse(data);

    if(matchedMessage.getName().isEmpty()) {
        qWarning("Received data could not be interpreted as any of the protocol's defined messages.");
        m_communicationLogTextEdit->appendPlainText(data);
        return;
    }

    m_communicationLogTextEdit->appendPlainText(matchedMessage.getPayloadSummary());
}

void TIODeviceWidget::sendRawBytes()
{
    m_communicationLogTextEdit->appendHtml(QStringLiteral("<b>Sent:</b>"));
    m_deviceModel->writeData(m_sendMessageEdit->text().toUtf8());
    m_communicationLogTextEdit->appendPlainText(m_sendMessageEdit->text());
}

void TIODeviceWidget::sendProtocolBytes()
{    
    if(!m_messageFormWidget->assignInputValues()) {
        qWarning("Message could not be sent because user input values were not valid.");
        return;
    }

    TMessage messageToBeSent = m_messageFormWidget->getMessage();
    const QByteArray & messageData = messageToBeSent.getData();
    if(messageData.length() == 0) {
        qWarning("Message could not be sent because data could not be formed.");
        TDialog::protocolMessageCouldNotBeFormed(this);
        return;
    }

    m_communicationLogTextEdit->appendHtml(QStringLiteral("<b>Sent:</b>"));
    m_deviceModel->writeData(messageData);
    m_communicationLogTextEdit->appendPlainText(messageToBeSent.getPayloadSummary());
}


void TIODeviceWidget::sendBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}

void TIODeviceWidget::sendFailed()
{
    TDialog::deviceSendFailedMessage(this);
}
