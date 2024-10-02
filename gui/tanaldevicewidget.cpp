#include "tanaldevicewidget.h"

#include <QGroupBox>
#include <QCheckBox>

#include "tdialog.h"

TAnalDeviceWidget::TAnalDeviceWidget(TAnalDeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel), m_senderModels(deviceModel->senderModels()), m_receiverModels(deviceModel->receiverModels()), m_actionModels(deviceModel->actionModels()), m_protocolContainer(protocolContainer)
{
    setWindowTitle(tr("Analytical Device - %1").arg(m_deviceModel->name()));

    for (int i = 0; i < m_receiverModels.length(); i++) {
        connect(m_receiverModels[i], &TReceiverModel::dataRead, this, [=](QByteArray data){ dataReceived(data, m_receiverModels[i]); });
    }

    m_communicationLogTextEdit = new QPlainTextEdit;
    m_communicationLogTextEdit->setReadOnly(true);

    m_paramWidget = new TConfigParamWidget(m_deviceModel->postInitParams());

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TAnalDeviceWidget::applyPostInitParam);

    QVBoxLayout * paramLayout = new QVBoxLayout;
    paramLayout->addWidget(m_paramWidget);
    paramLayout->addWidget(applyButton);

    QHBoxLayout * textParamLayout = new QHBoxLayout;

    textParamLayout->addWidget(m_communicationLogTextEdit);
    textParamLayout->addLayout(paramLayout);

    QGroupBox * textParamBox = new QGroupBox;
    textParamBox->setLayout(textParamLayout);

    QComboBox * senderComboBox = new QComboBox;

    for (int i = 0; i < m_senderModels.length(); i++) {
        senderComboBox->addItem(m_senderModels[i]->name());
    }
    senderComboBox->setCurrentIndex(0);
    senderChanged(0);
    connect(senderComboBox, &QComboBox::currentIndexChanged, this, &TAnalDeviceWidget::senderChanged);

    // Send message ComboBox
    m_sendMessageComboBox = new QComboBox;
    connect(m_sendMessageComboBox, &QComboBox::currentIndexChanged, this, &TAnalDeviceWidget::sendMessageChanged);

    m_noMessagesLabel = new QLabel(tr("Protocol doesn't contain any commands."));

    // Raw message row
    m_rawMessageEdit = new QLineEdit();
    connect(m_rawMessageEdit, &QLineEdit::textEdited, this, &TAnalDeviceWidget::validateRawInputValues);

    m_rawFormatComboBox = new QComboBox();
    m_rawFormatComboBox->addItem(tr("Hex"));
    m_rawFormatComboBox->addItem(tr("ASCII"));
    connect(m_rawFormatComboBox, &QComboBox::currentIndexChanged, this, &TAnalDeviceWidget::validateRawInputValues);

    m_rawMessageEditLayout = new QHBoxLayout();
    m_rawMessageEditLayout->setContentsMargins(0, 0, 0, 0);
    m_rawMessageEditLayout->addWidget(m_rawMessageEdit);
    m_rawMessageEditLayout->addWidget(m_rawFormatComboBox);
    m_rawMessageEditLayout->setStretch(0,4);
    m_rawMessageEditLayout->setStretch(1,1);

    m_sendProtocolComboBox = new QComboBox;

    m_sendButton = new QPushButton("Send");
    connect(m_sendButton, &QPushButton::clicked, this, &TAnalDeviceWidget::sendBytes);

    m_sendFormLayout = new QFormLayout;
    m_sendFormLayout->addRow(tr("Stream"), senderComboBox);
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
    sendLayout->addStretch();

    QGroupBox * sendMessageBox = new QGroupBox("Send data");
    sendMessageBox->setLayout(sendLayout);

    QComboBox * receiverComboBox = new QComboBox;

    for (int i = 0; i < m_receiverModels.length(); i++) {
        receiverComboBox->addItem(m_receiverModels[i]->name());
    }
    receiverComboBox->setCurrentIndex(0);
    receiverChanged(0);
    connect(receiverComboBox, &QComboBox::currentIndexChanged, this, &TAnalDeviceWidget::receiverChanged);

    // Receive data side
    m_receiveBytesEdit = new QLineEdit;
    QIntValidator * receiveBytesValidator = new QIntValidator;
    receiveBytesValidator->setBottom(1);
    m_receiveBytesEdit->setValidator(receiveBytesValidator);

    QPushButton * receiveButton = new QPushButton("Receive");
    receiveButton->setEnabled(false);
    connect(receiveButton, &QPushButton::clicked, this, &TAnalDeviceWidget::receiveBytes);

    connect(m_receiveBytesEdit, &QLineEdit::textChanged, this, [=](){receiveButton->setEnabled(m_receiveBytesEdit->hasAcceptableInput());});

    for (int i = 0; i < m_receiverModels.length(); i++) {
        connect(m_receiverModels[i], &TReceiverModel::readBusy, this, &TAnalDeviceWidget::receiveBusy);
    }

    QCheckBox * autoReceiveCheckbox = new QCheckBox;
    autoReceiveCheckbox->setChecked(false);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, this, &TAnalDeviceWidget::setAutoreceive);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, m_receiveBytesEdit, &QLineEdit::setDisabled);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, receiveButton, &QPushButton::setDisabled);

    QLabel * autoReceiveLabel = new QLabel(tr("Autoreceive"));

    QHBoxLayout * autoReceiveLayout = new QHBoxLayout;
    autoReceiveLayout->addStretch();
    autoReceiveLayout->addWidget(autoReceiveLabel);
    autoReceiveLayout->addWidget(autoReceiveCheckbox);

    m_receiveProtocolComboBox = new QComboBox;

    QFormLayout * receiveFormLayout = new QFormLayout;
    receiveFormLayout->addRow(tr("Stream"), receiverComboBox);
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

    QPushButton * runActionButton = new QPushButton(tr("Run"));
    connect(runActionButton, &QPushButton::clicked, this, &TAnalDeviceWidget::runAction);

    QPushButton * abortActionButton = new QPushButton(tr("Abort"));
    abortActionButton->setDisabled(true);
    connect(abortActionButton, &QPushButton::clicked, this, &TAnalDeviceWidget::abortAction);

    QLabel * actionLabel = new QLabel(tr("Action"));

    QComboBox * actionComboBox = new QComboBox;

    for (int i = 0; i < m_actionModels.length(); i++) {
        actionComboBox->addItem(m_actionModels[i]->name());
        connect(m_actionModels[i], &TAnalActionModel::started, actionComboBox, [=](){ actionComboBox->setDisabled(true); runActionButton->setDisabled(true); abortActionButton->setDisabled(false); });
        connect(m_actionModels[i], &TAnalActionModel::finished, actionComboBox, [=](){ actionComboBox->setDisabled(false); runActionButton->setDisabled(!m_currentActionModel->isEnabled()); abortActionButton->setDisabled(true); });
    }
    actionComboBox->setCurrentIndex(0);
    actionChanged(0);
    connect(actionComboBox, &QComboBox::currentIndexChanged, this, &TAnalDeviceWidget::actionChanged);
    connect(actionComboBox, &QComboBox::currentIndexChanged, this, [=](int index){ runActionButton->setDisabled(!m_currentActionModel->isEnabled()); });

    QHBoxLayout * actionLayout = new QHBoxLayout;
    actionLayout->addWidget(actionLabel);
    actionLayout->addWidget(actionComboBox);
    actionLayout->addWidget(runActionButton);
    actionLayout->addWidget(abortActionButton);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(textParamBox);
    layout->addLayout(sendReceiveLayout);
    layout->addLayout(actionLayout);

    for (int i = 0; i < m_receiverModels.length(); i++) {
        connect(m_receiverModels[i], &TReceiverModel::readFailed, this, &TAnalDeviceWidget::receiveFailed);
    }

    updateDisplayedProtocols();
    connect(m_sendProtocolComboBox, &QComboBox::currentIndexChanged, this, &TAnalDeviceWidget::sendProtocolChanged);
    connect(m_protocolContainer, &TProtocolContainer::protocolsUpdated, this, &TAnalDeviceWidget::updateDisplayedProtocols);

    validateRawInputValues();

    setLayout(layout);
}

TAnalDeviceWidget::~TAnalDeviceWidget() {
    delete m_messageFormManager;
}

bool TAnalDeviceWidget::validateRawInputValues() {
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

void TAnalDeviceWidget::updateDisplayedProtocols() {
    m_sendProtocolComboBox->clear();
    m_receiveProtocolComboBox->clear();

    m_sendProtocolComboBox->addItem("raw data");
    m_receiveProtocolComboBox->addItem("raw data");

    for(int i = 0; i < m_protocolContainer->count(); i++) {
        m_sendProtocolComboBox->addItem(m_protocolContainer->at(i).getName());
        m_receiveProtocolComboBox->addItem(m_protocolContainer->at(i).getName());
    }
}

void TAnalDeviceWidget::sendProtocolChanged(int index)
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

void TAnalDeviceWidget::sendMessageChanged(int index)
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

void TAnalDeviceWidget::senderChanged(int index)
{
    m_currentSenderModel = m_senderModels[index];
}

void TAnalDeviceWidget::receiverChanged(int index)
{
    m_currentReceiverModel = m_receiverModels[index];
}

void TAnalDeviceWidget::actionChanged(int index)
{
    m_currentActionModel = m_actionModels[index];
}

void TAnalDeviceWidget::runAction()
{
    emit m_currentActionModel->run();
}

void TAnalDeviceWidget::abortAction()
{
    emit m_currentActionModel->abort();
}

bool TAnalDeviceWidget::applyPostInitParam()
{
    TConfigParam param = m_deviceModel->setPostInitParams(m_paramWidget->param());
    m_paramWidget->setParam(param);

    if (param.getState(true) == TConfigParam::TState::TError) {
        qWarning("TIODevice parameters not set due to error state!");
        return false;
    };

    return true;
}

void TAnalDeviceWidget::setAutoreceive(bool enabled)
{
    if (enabled) {
        m_currentReceiverModel->enableAutoRead();
    }
    else {
        m_currentReceiverModel->disableAutoRead();
    }
}

void TAnalDeviceWidget::receiveBytes()
{
    m_currentReceiverModel->readData(m_receiveBytesEdit->text().toInt());
}

void TAnalDeviceWidget::receiveBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}

void TAnalDeviceWidget::receiveFailed()
{
    TDialog::deviceReceiveFailedMessage(this);
}

void TAnalDeviceWidget::dataReceived(QByteArray data, TAnalStreamReceiverModel * receiverModel)
{
    m_communicationLogTextEdit->appendHtml(QString("<b>Received: (%1)</b>").arg(receiverModel->name()));

    QString selectedProtocolName = m_receiveProtocolComboBox->currentText();

    if(selectedProtocolName == "raw data") {
        m_communicationLogTextEdit->appendPlainText(byteArraytoHumanReadableString(data));
        return;
    }

    bool protocolFound;
    TProtocol selectedReceiveProtocol = m_protocolContainer->getByName(selectedProtocolName, &protocolFound);

    if(!protocolFound) {
        qWarning("Unknown protocol selected, could not interpret message.");
        m_communicationLogTextEdit->appendPlainText(byteArraytoHumanReadableString(data));
        return;
    }

    TMessage matchedMessage = selectedReceiveProtocol.tryMatchResponse(data);

    if(matchedMessage.getName().isEmpty()) {
        qWarning("Received data could not be interpreted as any of the protocol's defined messages.");
        m_communicationLogTextEdit->appendPlainText(byteArraytoHumanReadableString(data));
        return;
    }

    m_communicationLogTextEdit->appendPlainText(matchedMessage.getPayloadSummary());
}

QString TAnalDeviceWidget::byteArraytoHumanReadableString(const QByteArray & byteArray)
{
    static QRegularExpression nonAsciiRegExp("[^ -~]");
    bool isHumanReadable = !((QString)byteArray).contains(nonAsciiRegExp);

    return isHumanReadable ? byteArray : ("0x" + byteArray.toHex());
}

void TAnalDeviceWidget::sendBytes()
{
    if(m_selectedProtocol.getName().isEmpty()) {
        sendRawBytes();
    }
    else {
        sendProtocolBytes();
    }
}

void TAnalDeviceWidget::sendRawBytes()
{
    if(!validateRawInputValues()) {
        TDialog::parameterValueInvalid(this, tr("payload"));
        return;
    }

    QByteArray dataToWrite = m_rawMessageEdit->text().toUtf8();

    bool isAscii = m_rawFormatComboBox->currentIndex();
    if(!isAscii) {
        dataToWrite = QByteArray::fromHex(dataToWrite);
    }

    m_currentSenderModel->writeData(dataToWrite);

    m_communicationLogTextEdit->appendHtml(QString("<b>Sent: (%1)</b>").arg(m_currentSenderModel->name()));
    m_communicationLogTextEdit->appendPlainText(byteArraytoHumanReadableString(dataToWrite));
}

void TAnalDeviceWidget::sendProtocolBytes()
{
    if(!m_messageFormManager->assignInputValues()) {
        qWarning("Message could not be sent because user input values were not valid.");
        return;
    }

    TMessage messageToBeSent = m_messageFormManager->getMessage();
    const QByteArray & messageData = messageToBeSent.getData();
    if(messageData.length() == 0) {
        qWarning("Message could not be sent because data could not be formed.");
        TDialog::protocolMessageCouldNotBeFormed(this);
        return;
    }

    m_communicationLogTextEdit->appendHtml(QStringLiteral("<b>Sent:</b>"));
    m_currentSenderModel->writeData(messageData);
    m_communicationLogTextEdit->appendPlainText(messageToBeSent.getPayloadSummary());
}

void TAnalDeviceWidget::sendBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}

void TAnalDeviceWidget::sendFailed()
{
    TDialog::deviceSendFailedMessage(this);
}
