#include "tiodevicewidget.h"

#include <QLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QCoreApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>

#include "tconfigparamwidget.h"
#include "tdialog.h"
#include "tfilenameedit.h"


TIODeviceWidget::TIODeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel), m_senderModel(m_deviceModel->senderModel()), m_receiverModel(m_deviceModel->receiverModel()), m_protocolContainer(protocolContainer)
{
    setWindowTitle(tr("IO Device - %1").arg(m_deviceModel->name()));
    setFocusPolicy(Qt::ClickFocus);

    m_communicationLogTextEdit = new QPlainTextEdit;
    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    m_communicationLogTextEdit->setFont(f);
    m_communicationLogTextEdit->setReadOnly(true);
    
    m_paramWidget = new TConfigParamWidget(m_deviceModel->postInitParams());

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TIODeviceWidget::applyPostInitParam);

    QVBoxLayout * paramLayout = new QVBoxLayout;
    paramLayout->addWidget(m_paramWidget);
    paramLayout->addWidget(applyButton);

    QHBoxLayout * textParamLayout = new QHBoxLayout;

    QVBoxLayout * comLogLayout = new QVBoxLayout;

    comLogLayout->addWidget(m_communicationLogTextEdit);

    QHBoxLayout * comLogSettingsLayout = new QHBoxLayout;

    m_logFormat = new QComboBox;
    m_logFormat->addItem("Show only hexadecimal values");
    m_logFormat->addItem("Show human-readable string when possible");
    comLogSettingsLayout->addWidget(m_logFormat);

    QPushButton * clearButton = new QPushButton;
    clearButton->setText("Clear");
    connect(clearButton, &QPushButton::clicked, m_communicationLogTextEdit, &QPlainTextEdit::clear);
    comLogSettingsLayout->addWidget(clearButton);

    comLogLayout->addLayout(comLogSettingsLayout);

    textParamLayout->addLayout(comLogLayout);
    textParamLayout->addLayout(paramLayout);

    QGroupBox * textParamBox = new QGroupBox;
    textParamBox->setLayout(textParamLayout);

    // Send message ComboBox
    m_sendMessageComboBox = new QComboBox;
    connect(m_sendMessageComboBox, &QComboBox::currentIndexChanged, this, &TIODeviceWidget::sendMessageChanged);

    m_noMessagesLabel = new QLabel(tr("Protocol doesn't contain any commands."));

    // Raw message row
    m_rawMessageEdit = new QLineEdit();
    connect(m_rawMessageEdit, &QLineEdit::textEdited, this, &TIODeviceWidget::validateRawInputValues);

    m_rawFormatComboBox = new QComboBox();
    m_rawFormatComboBox->addItem(tr("Hex"));
    m_rawFormatComboBox->addItem(tr("ASCII"));
    connect(m_rawFormatComboBox, &QComboBox::currentIndexChanged, this, &TIODeviceWidget::validateRawInputValues);

    m_rawMessageEditLayout = new QHBoxLayout();
    m_rawMessageEditLayout->setContentsMargins(0, 0, 0, 0);
    m_rawMessageEditLayout->addWidget(m_rawMessageEdit);
    m_rawMessageEditLayout->addWidget(m_rawFormatComboBox);
    m_rawMessageEditLayout->setStretch(0,4);
    m_rawMessageEditLayout->setStretch(1,1);

    m_sendProtocolComboBox = new QComboBox;

    m_sendButton = new QPushButton("Send");
    connect(m_sendButton, &QPushButton::clicked, this, &TIODeviceWidget::sendBytes);

    QGroupBox * sendFileGroupBox = new QGroupBox("Send file");

    QLayout * sendFileLayout = new QVBoxLayout();

    TFileNameEdit * sendFileEdit = new TFileNameEdit(QFileDialog::ExistingFile);
    QPushButton * sendFileButton = new QPushButton("Send");
    connect(sendFileButton, &QPushButton::clicked, this, [=](){ sendFile(sendFileEdit->text()); });

    sendFileLayout->addWidget(sendFileEdit);
    sendFileLayout->addWidget(sendFileButton);

    sendFileGroupBox->setLayout(sendFileLayout);

    m_sendFormLayout = new QFormLayout;
    m_sendFormLayout->addRow(tr("Protocol"), m_sendProtocolComboBox);
    m_sendFormLayout->addRow(tr("Message"), m_sendMessageComboBox);
    m_sendFormLayout->setRowVisible(m_sendMessageComboBox, false);
    m_sendFormLayout->addRow(tr("Message"), m_noMessagesLabel);
    m_sendFormLayout->setRowVisible(m_noMessagesLabel, false);
    m_sendFormLayout->addRow(tr("Payload"), m_rawMessageEditLayout);
    m_sendFormLayout->addWidget(m_sendButton);
    
    m_messageFormManager = new TMessageFormManager(m_sendFormLayout, 3);

    QVBoxLayout * sendLayout = new QVBoxLayout;
    sendLayout->addLayout(m_sendFormLayout);
    sendLayout->addWidget(sendFileGroupBox);
    sendLayout->addStretch();

    QGroupBox * sendMessageBox = new QGroupBox("Send data");
    sendMessageBox->setLayout(sendLayout);

    // Receive data side
    m_receiveBytesEdit = new QLineEdit;
    QIntValidator * receiveBytesValidator = new QIntValidator;
    receiveBytesValidator->setBottom(1);
    m_receiveBytesEdit->setValidator(receiveBytesValidator);

    QPushButton * receiveButton = new QPushButton("Receive");
    receiveButton->setEnabled(false);
    connect(receiveButton, &QPushButton::clicked, this, &TIODeviceWidget::receiveBytes);
    connect(m_receiveBytesEdit, &QLineEdit::textChanged, this, [=](){receiveButton->setEnabled(m_receiveBytesEdit->hasAcceptableInput());});

    connect(m_receiverModel, &TReceiverModel::readBusy, this, &TIODeviceWidget::receiveBusy);
    connect(m_receiverModel, &TReceiverModel::readFailed, this, &TIODeviceWidget::receiveFailed);
    connect(m_receiverModel, &TReceiverModel::dataRead, this, &TIODeviceWidget::dataReceived);

    connect(m_senderModel, &TSenderModel::writeBusy, this, &TIODeviceWidget::sendBusy);
    connect(m_senderModel, &TSenderModel::writeFailed, this, &TIODeviceWidget::sendFailed);
    connect(m_senderModel, &TSenderModel::dataWritten, this, &TIODeviceWidget::dataSent);

    QCheckBox * autoReceiveCheckbox = new QCheckBox;
    autoReceiveCheckbox->setChecked(false);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, this, &TIODeviceWidget::setAutoreceive);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, m_receiveBytesEdit, &QLineEdit::setDisabled);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, receiveButton, &QPushButton::setDisabled);

    QLabel * autoReceiveLabel = new QLabel(tr("Autoreceive"));

    QHBoxLayout * autoReceiveLayout = new QHBoxLayout;
    autoReceiveLayout->addStretch();
    autoReceiveLayout->addWidget(autoReceiveLabel);
    autoReceiveLayout->addWidget(autoReceiveCheckbox);

    m_receiveProtocolComboBox = new QComboBox;

    QFormLayout * receiveFormLayout = new QFormLayout;
    receiveFormLayout->addRow(tr("Protocol"), m_receiveProtocolComboBox);
    receiveFormLayout->addRow(tr("Bytes"), m_receiveBytesEdit);
    receiveFormLayout->addWidget(receiveButton);

    QVBoxLayout * receiveLayout = new QVBoxLayout;
    receiveLayout->addLayout(receiveFormLayout);
    receiveLayout->addLayout(autoReceiveLayout);
    receiveLayout->addStretch();

    QGroupBox * receiveMessageBox = new QGroupBox("Receive data");
    receiveMessageBox->setLayout(receiveLayout);

    QHBoxLayout * sendReceiveLayout = new QHBoxLayout;
    sendReceiveLayout->addWidget(sendMessageBox);
    sendReceiveLayout->addWidget(receiveMessageBox);
    sendReceiveLayout->setStretch(0,1);
    sendReceiveLayout->setStretch(1,1);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(textParamBox);
    layout->addLayout(sendReceiveLayout);

    connect(m_receiverModel, &TReceiverModel::readFailed, this, &TIODeviceWidget::receiveFailed);
    updateDisplayedProtocols();
    connect(m_sendProtocolComboBox, &QComboBox::currentIndexChanged, this, &TIODeviceWidget::sendProtocolChanged);
    connect(m_protocolContainer, &TProtocolContainer::protocolsUpdated, this, &TIODeviceWidget::updateDisplayedProtocols);

    validateRawInputValues();

    setLayout(layout);
}

TIODeviceWidget::~TIODeviceWidget() {
    delete m_messageFormManager;
}

bool TIODeviceWidget::validateRawInputValues() {
    bool iok;

    bool isAscii = m_rawFormatComboBox->currentIndex();
    if(isAscii) {
        static QRegularExpression asciiRegExp("^([\\x00-\\x7F])+$");
        iok = asciiRegExp.match(m_rawMessageEdit->text()).hasMatch();
    }
    else {
        static QRegularExpression hexRegExp("^([A-Fa-f0-9]|([A-Fa-f0-9]{2})+)$");
        iok = hexRegExp.match(m_rawMessageEdit->text()).hasMatch();
    }

    m_rawMessageEdit->setStyleSheet(iok ? "background-color: white;" : "background-color: rgba(255, 0, 0, 0.3);");
    return iok;
}

void TIODeviceWidget::updateDisplayedProtocols() {
    m_sendProtocolComboBox->clear();
    m_receiveProtocolComboBox->clear();

    m_sendProtocolComboBox->addItem("No protocol");
    m_receiveProtocolComboBox->addItem("No protocol");

    for(int i = 0; i < m_protocolContainer->count(); i++) {
        m_sendProtocolComboBox->addItem(m_protocolContainer->at(i).getName());
        m_receiveProtocolComboBox->addItem(m_protocolContainer->at(i).getName());
    }
}

void TIODeviceWidget::sendProtocolChanged(int index)
{
    m_messageFormManager->clearRows();

    if(index < 1) {
        m_selectedProtocol = TProtocol();

        m_sendFormLayout->setRowVisible(m_rawMessageEditLayout, true);
        m_sendFormLayout->setRowVisible(m_sendMessageComboBox, false);
        m_sendFormLayout->setRowVisible(m_noMessagesLabel, false);
        m_sendButton->setEnabled(true);
        return;
    }

    bool protocolFound;
    m_selectedProtocol = m_protocolContainer->getByName(m_sendProtocolComboBox->currentText(), &protocolFound);

    if(!protocolFound) {
        qWarning("Unknown protocol selected, maybe the user removed it?");
        return;
    }

    m_sendMessageComboBox->clear();    
    m_sendFormLayout->setRowVisible(m_rawMessageEditLayout, false);

    for(const TMessage & message : m_selectedProtocol.getMessages()) {
        if(message.isResponse()) {
            continue;
        }

        m_sendMessageComboBox->addItem(message.getName());
    }

    if(m_sendMessageComboBox->count() == 0) {
        m_sendFormLayout->setRowVisible(m_noMessagesLabel, true);
        m_sendFormLayout->setRowVisible(m_sendMessageComboBox, false);
        m_sendButton->setEnabled(false);
        return;
    }

    m_sendFormLayout->setRowVisible(m_noMessagesLabel, false);
    m_sendFormLayout->setRowVisible(m_sendMessageComboBox, true);
    m_sendButton->setEnabled(true);
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
        m_messageFormManager->clearRows();
        return;
    }

    m_messageFormManager->setMessage(m_selectedMessage);
}

bool TIODeviceWidget::applyPostInitParam()
{
    TConfigParam param = m_deviceModel->setPostInitParams(m_paramWidget->param());
    m_paramWidget->setParam(param);

    if (param.getState(true) == TConfigParam::TState::TError) {
        qWarning("TIODevice parameters not set due to error state!");
        return false;
    };

    return true;
}

void TIODeviceWidget::setAutoreceive(bool enabled)
{
    if (enabled) {
        m_receiverModel->enableAutoRead();
    }
    else {
        m_receiverModel->disableAutoRead();
    }
}

void TIODeviceWidget::receiveBytes()
{
    m_receiverModel->readData(m_receiveBytesEdit->text().toInt());
}

void TIODeviceWidget::receiveBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}

void TIODeviceWidget::receiveFailed()
{
    TDialog::deviceReceiveFailedMessage(this);
}

void TIODeviceWidget::dataReceived(QByteArray data)
{
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");

    QString selectedProtocolName = m_receiveProtocolComboBox->currentText();

    if(selectedProtocolName == "No protocol") {
        m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received " + QString::number(data.size()) + " B</b>"));
        m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + byteArraytoHumanReadableString(data) + "</div>");
        return;
    }

    bool protocolFound;
    TProtocol selectedReceiveProtocol = m_protocolContainer->getByName(selectedProtocolName, &protocolFound);

    if(!protocolFound) {
        qWarning("Unknown protocol selected, could not interpret message.");
        m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received " + QString::number(data.size()) + " B</b>"));
        m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + byteArraytoHumanReadableString(data) + "</div>");
        return;
    }

    TMessage matchedMessage = selectedReceiveProtocol.tryMatchResponse(data);

    if(matchedMessage.getName().isEmpty()) {
        qWarning("Received data could not be interpreted as any of the protocol's defined messages.");
        m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received " + QString::number(data.size()) + " B</b>"));
        m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + byteArraytoHumanReadableString(data) + "</div>");
        return;
    }

    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received:</b>"));
    m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + matchedMessage.getPayloadSummary() + "</div>");
}

QString TIODeviceWidget::byteArraytoHumanReadableString(const QByteArray & byteArray)
{
    static QRegularExpression nonAsciiRegExp("[^ -~]");
    bool isHumanReadable = !((QString)byteArray).contains(nonAsciiRegExp);

    if(m_logFormat->currentIndex() == 0){
        return (byteArray.toHex(' '));
    } else {
        return isHumanReadable ? "<i>\"" + QString(byteArray).toHtmlEscaped() + "\"</i>" : (byteArray.toHex(' '));
    }

}

void TIODeviceWidget::sendBytes()
{
    if(m_selectedProtocol.getName().isEmpty()) {
        sendRawBytes();
    }
    else {
        sendProtocolBytes();
    }
}

void TIODeviceWidget::sendRawBytes()
{
    if(!validateRawInputValues()) {
        TDialog::parameterValueInvalid(this, tr("payload"));
        return;
    }

    QByteArray dataToWrite = m_rawMessageEdit->text().toUtf8();
    dataToWrite.replace("\\n", "\n");
    dataToWrite.replace("\\r", "\r");

    bool isAscii = m_rawFormatComboBox->currentIndex();
    if(!isAscii) {
        dataToWrite = QByteArray::fromHex(dataToWrite);
    }

    m_senderModel->writeData(dataToWrite);
}

void TIODeviceWidget::sendProtocolBytes()
{
    if(!m_messageFormManager->assignInputValues()) {
        qWarning("Message could not be sent because user input values were not valid.");
        return;
    }

    m_messageToBeSent = m_messageFormManager->getMessage();
    const QByteArray & messageData = m_messageToBeSent.getData();
    if(messageData.length() == 0) {
        qWarning("Message could not be sent because data could not be formed.");
        TDialog::protocolMessageCouldNotBeFormed(this);
        return;
    }

    m_senderModel->writeData(messageData);    
}

void TIODeviceWidget::sendFile(QString fileName)
{
    QFile file(fileName);

    if (!file.exists()) {
        qWarning("File cannot be sent because it does not exist.");
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("File cannot be sent because it failed to open.");
        return;
    }

    m_senderModel->writeData(file.readAll());

    file.close();
}

void TIODeviceWidget::sendBusy()
{
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");
    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Failed to send data: device busy</b>"));

    TDialog::deviceFailedBusyMessage(this);
}

void TIODeviceWidget::sendFailed()
{
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");
    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Failed to send data</b>"));

    TDialog::deviceSendFailedMessage(this);
}

void TIODeviceWidget::dataSent(QByteArray data)
{
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");
    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Sent " + QString::number(data.size()) + " B</b>"));

    if(data.size() > 512) {
        m_communicationLogTextEdit->appendHtml("<div style=\"color:blue\">Payload is too big to be shown...</div>");
    }
    else {
        if(data == m_messageToBeSent.getData()) {
            m_communicationLogTextEdit->appendHtml("<div style=\"color:blue\">" + m_messageToBeSent.getPayloadSummary() + "</div>");
        }
        else {
            m_communicationLogTextEdit->appendHtml("<div style=\"color:blue\">" + byteArraytoHumanReadableString(data) + "</div>");
        }
    }
}
