#include "tcommunicationdevicewidget.h"

#include <QGroupBox>
#include <QCheckBox>

#include "../../tdialog.h"
#include "widgets/tfilenameedit.h"

TCommunicationDeviceWidget::TCommunicationDeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel), m_protocolContainer(protocolContainer)
{
    setWindowTitle(tr("IO Device - %1").arg(m_deviceModel->name()));

    m_senderModels.append(deviceModel->senderModel());
    m_receiverModels.append(deviceModel->receiverModel());

    init();
}

TCommunicationDeviceWidget::TCommunicationDeviceWidget(TAnalDeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel), m_senderModels(deviceModel->senderModels()), m_receiverModels(deviceModel->receiverModels()), m_actionModels(deviceModel->actionModels()), m_protocolContainer(protocolContainer)
{
    setWindowTitle(tr("Analytical Device - %1").arg(m_deviceModel->name()));

    init();
}

void TCommunicationDeviceWidget::init() {
    setFocusPolicy(Qt::ClickFocus);

    m_communicationLogTextEdit = new QPlainTextEdit;
    QFont f("unexistent");
    f.setStyleHint(QFont::Monospace);
    m_communicationLogTextEdit->setFont(f);
    m_communicationLogTextEdit->setReadOnly(true);

    m_paramWidget = new TConfigParamWidget(m_deviceModel->postInitParams());

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TCommunicationDeviceWidget::applyPostInitParam);

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
    connect(clearButton, &QPushButton::clicked, this, [=](){ for (int i = 0; i < m_receivedData.length(); i++) m_receivedData[i]->clear(); });
    comLogSettingsLayout->addWidget(clearButton);

    comLogLayout->addLayout(comLogSettingsLayout);

    textParamLayout->addLayout(comLogLayout);
    textParamLayout->addLayout(paramLayout);

    QGroupBox * textParamBox = new QGroupBox;
    textParamBox->setLayout(textParamLayout);

    QComboBox * senderComboBox = nullptr;
    if (m_senderModels.length() > 1 || !m_senderModels[0]->name().isEmpty()) {
        senderComboBox = new QComboBox;

        for (int i = 0; i < m_senderModels.length(); i++) {
            senderComboBox->addItem(m_senderModels[i]->name());
            senderComboBox->setItemData(i, m_senderModels[i]->info(), Qt::ToolTipRole);
        }
        senderComboBox->setCurrentIndex(0);

        connect(senderComboBox, &QComboBox::currentIndexChanged, this, &TCommunicationDeviceWidget::senderChanged);
    }
    senderChanged(0);

    // Send message ComboBox
    m_sendMessageComboBox = new QComboBox;
    connect(m_sendMessageComboBox, &QComboBox::currentIndexChanged, this, &TCommunicationDeviceWidget::sendMessageChanged);

    m_noMessagesLabel = new QLabel(tr("Protocol doesn't contain any commands."));

    // Raw message row
    m_rawMessageEdit = new QLineEdit();
    connect(m_rawMessageEdit, &QLineEdit::textEdited, this, &TCommunicationDeviceWidget::validateRawInputValues);

    m_rawFormatComboBox = new QComboBox();
    m_rawFormatComboBox->addItem(tr("Hex"));
    m_rawFormatComboBox->addItem(tr("ASCII"));
    connect(m_rawFormatComboBox, &QComboBox::currentIndexChanged, this, &TCommunicationDeviceWidget::validateRawInputValues);

    m_rawMessageEditLayout = new QHBoxLayout();
    m_rawMessageEditLayout->setContentsMargins(0, 0, 0, 0);
    m_rawMessageEditLayout->addWidget(m_rawMessageEdit);
    m_rawMessageEditLayout->addWidget(m_rawFormatComboBox);
    m_rawMessageEditLayout->setStretch(0,4);
    m_rawMessageEditLayout->setStretch(1,1);

    m_sendProtocolComboBox = new QComboBox;

    m_sendButton = new QPushButton("Send");
    connect(m_sendButton, &QPushButton::clicked, this, &TCommunicationDeviceWidget::sendBytes);

    QGroupBox * sendFileGroupBox = new QGroupBox("Send file");

    QLayout * sendFileLayout = new QVBoxLayout();

    TFileNameEdit * sendFileEdit = new TFileNameEdit(QFileDialog::ExistingFile);
    QPushButton * sendFileButton = new QPushButton("Send");
    connect(sendFileButton, &QPushButton::clicked, this, [=](){ sendFile(sendFileEdit->text()); });

    sendFileLayout->addWidget(sendFileEdit);
    sendFileLayout->addWidget(sendFileButton);

    sendFileGroupBox->setLayout(sendFileLayout);

    m_sendFormLayout = new QFormLayout;    
    if (senderComboBox)
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
    sendLayout->addWidget(sendFileGroupBox);
    sendLayout->addStretch();

    QGroupBox * sendMessageBox = new QGroupBox("Send data");
    sendMessageBox->setLayout(sendLayout);

    QComboBox * receiverComboBox = nullptr;

    if (m_receiverModels.length() > 1 || !m_receiverModels[0]->name().isEmpty()) {
        receiverComboBox = new QComboBox;

        for (int i = 0; i < m_receiverModels.length(); i++) {
            receiverComboBox->addItem(m_receiverModels[i]->name());
            receiverComboBox->setItemData(i, m_receiverModels[i]->info(), Qt::ToolTipRole);
        }

        receiverComboBox->setCurrentIndex(0);
        connect(receiverComboBox, &QComboBox::currentIndexChanged, this, &TCommunicationDeviceWidget::receiverChanged);
    }
    receiverChanged(0);

    // Receive data side
    m_receiveBytesEdit = new QLineEdit;
    QIntValidator * receiveBytesValidator = new QIntValidator;
    receiveBytesValidator->setBottom(1);
    m_receiveBytesEdit->setValidator(receiveBytesValidator);

    QPushButton * receiveButton = new QPushButton("Receive");
    receiveButton->setEnabled(false);
    connect(receiveButton, &QPushButton::clicked, this, &TCommunicationDeviceWidget::receiveBytes);
    connect(m_receiveBytesEdit, &QLineEdit::textChanged, this, [=](){receiveButton->setEnabled(m_receiveBytesEdit->hasAcceptableInput());});

    for (int i = 0; i < m_receiverModels.length(); i++) {
        connect(m_receiverModels[i], &TReceiverModel::readBusy, this, &TCommunicationDeviceWidget::receiveBusy);
        connect(m_receiverModels[i], &TReceiverModel::readFailed, this, &TCommunicationDeviceWidget::receiveFailed);
        connect(m_receiverModels[i], &TReceiverModel::dataRead, this, [=](QByteArray data){ dataReceived(data, m_receiverModels[i]); });
    }

    for (int i = 0; i < m_senderModels.length(); i++) {
        connect(m_senderModels[i], &TSenderModel::writeBusy, this, &TCommunicationDeviceWidget::sendBusy);
        connect(m_senderModels[i], &TSenderModel::writeFailed, this, &TCommunicationDeviceWidget::sendFailed);
        connect(m_senderModels[i], &TSenderModel::dataWritten, this, [=](QByteArray data){ dataSent(data, m_senderModels[i]); });
    }

    QCheckBox * autoReceiveCheckbox = new QCheckBox;
    autoReceiveCheckbox->setChecked(false);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, this, &TCommunicationDeviceWidget::setAutoreceive);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, m_receiveBytesEdit, &QLineEdit::setDisabled);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, receiveButton, &QPushButton::setDisabled);

    QLabel * autoReceiveLabel = new QLabel(tr("Autoreceive"));

    QHBoxLayout * autoReceiveLayout = new QHBoxLayout;
    autoReceiveLayout->addStretch();
    autoReceiveLayout->addWidget(autoReceiveLabel);
    autoReceiveLayout->addWidget(autoReceiveCheckbox);

    m_receiveProtocolComboBox = new QComboBox;

    QFormLayout * receiveFormLayout = new QFormLayout;
    if (receiverComboBox)
        receiveFormLayout->addRow(tr("Stream"), receiverComboBox);
    receiveFormLayout->addRow(tr("Protocol"), m_receiveProtocolComboBox);
    receiveFormLayout->addRow(tr("Bytes"), m_receiveBytesEdit);
    receiveFormLayout->addWidget(receiveButton);

    QGroupBox * receiveFileGroupBox = new QGroupBox("Save current stream to file (raw)");

    QLayout * receiveFileLayout = new QVBoxLayout();

    TFileNameEdit * receiveFileEdit = new TFileNameEdit(QFileDialog::AnyFile);
    QPushButton * receiveFileButton = new QPushButton("Save");
    connect(receiveFileButton, &QPushButton::clicked, this, [=](){ receiveFile(receiveFileEdit->text()); });

    receiveFileLayout->addWidget(receiveFileEdit);
    receiveFileLayout->addWidget(receiveFileButton);

    receiveFileGroupBox->setLayout(receiveFileLayout);

    QVBoxLayout * receiveLayout = new QVBoxLayout;
    receiveLayout->addLayout(receiveFormLayout);
    receiveLayout->addLayout(autoReceiveLayout);
    receiveLayout->addWidget(receiveFileGroupBox);
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

    if (!m_actionModels.empty()) {        
        QPushButton * runActionButton = new QPushButton(tr("Run"));
        connect(runActionButton, &QPushButton::clicked, this, &TCommunicationDeviceWidget::runAction);

        QPushButton * abortActionButton = new QPushButton(tr("Abort"));
        abortActionButton->setDisabled(true);
        connect(abortActionButton, &QPushButton::clicked, this, &TCommunicationDeviceWidget::abortAction);

        QLabel * actionLabel = new QLabel(tr("Action"));

        QComboBox * actionComboBox = new QComboBox;

        for (int i = 0; i < m_actionModels.length(); i++) {
            actionComboBox->addItem(m_actionModels[i]->name());
            connect(m_actionModels[i], &TAnalActionModel::started, actionComboBox, [=](){ actionComboBox->setDisabled(true); runActionButton->setDisabled(true); abortActionButton->setDisabled(false); });
            connect(m_actionModels[i], &TAnalActionModel::finished, actionComboBox, [=](){ actionComboBox->setDisabled(false); runActionButton->setDisabled(!m_currentActionModel->isEnabled()); abortActionButton->setDisabled(true); });
        }

        actionComboBox->setCurrentIndex(0);
        actionChanged(0);
        connect(actionComboBox, &QComboBox::currentIndexChanged, this, &TCommunicationDeviceWidget::actionChanged);
        connect(actionComboBox, &QComboBox::currentIndexChanged, this, [=](int index){ runActionButton->setDisabled(!m_currentActionModel->isEnabled()); });

        QHBoxLayout * actionLayout = new QHBoxLayout;
        actionLayout->addWidget(actionLabel);
        actionLayout->addWidget(actionComboBox);
        actionLayout->addWidget(runActionButton);
        actionLayout->addWidget(abortActionButton);

        layout->addLayout(actionLayout);
    }

    updateDisplayedProtocols();
    connect(m_sendProtocolComboBox, &QComboBox::currentIndexChanged, this, &TCommunicationDeviceWidget::sendProtocolChanged);
    connect(m_protocolContainer, &TProtocolContainer::protocolsUpdated, this, &TCommunicationDeviceWidget::updateDisplayedProtocols);

    validateRawInputValues();

    setLayout(layout);

    for (int i = 0; i < m_receiverModels.length(); i++)
        m_receivedData.append(new QByteArray);
}

TCommunicationDeviceWidget::~TCommunicationDeviceWidget() {
    delete m_messageFormManager;

    for (int i = 0; i < m_receiverModels.length(); i++)
        delete m_receivedData[i];
}

bool TCommunicationDeviceWidget::validateRawInputValues() {
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

void TCommunicationDeviceWidget::updateDisplayedProtocols() {
    m_sendProtocolComboBox->clear();
    m_receiveProtocolComboBox->clear();

    m_sendProtocolComboBox->addItem("raw data");
    m_receiveProtocolComboBox->addItem("raw data");

    for(int i = 0; i < m_protocolContainer->count(); i++) {
        m_sendProtocolComboBox->addItem(m_protocolContainer->at(i)->name());
        m_receiveProtocolComboBox->addItem(m_protocolContainer->at(i)->name());
    }
}

void TCommunicationDeviceWidget::sendProtocolChanged(int index)
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

void TCommunicationDeviceWidget::sendMessageChanged(int index)
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

void TCommunicationDeviceWidget::senderChanged(int index)
{
    m_currentSenderModel = m_senderModels[index];
}

void TCommunicationDeviceWidget::receiverChanged(int index)
{
    m_currentReceiverModel = m_receiverModels[index];
}

void TCommunicationDeviceWidget::actionChanged(int index)
{
    m_currentActionModel = m_actionModels[index];
}

void TCommunicationDeviceWidget::runAction()
{
    emit m_currentActionModel->run();
}

void TCommunicationDeviceWidget::abortAction()
{
    emit m_currentActionModel->abort();
}

bool TCommunicationDeviceWidget::applyPostInitParam()
{
    TConfigParam param = m_deviceModel->setPostInitParams(m_paramWidget->param());
    m_paramWidget->setParam(param);

    if (param.getState(true) == TConfigParam::TState::TError) {
        qWarning("Device parameters not set due to error state!");
        return false;
    };

    return true;
}

void TCommunicationDeviceWidget::setAutoreceive(bool enabled)
{
    if (enabled) {
        m_currentReceiverModel->enableAutoRead();
    }
    else {
        m_currentReceiverModel->disableAutoRead();
    }
}

void TCommunicationDeviceWidget::receiveBytes()
{
    m_currentReceiverModel->readData(m_receiveBytesEdit->text().toInt());
}

void TCommunicationDeviceWidget::receiveBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}

void TCommunicationDeviceWidget::receiveFailed()
{
    TDialog::deviceReceiveFailedMessage(this);
}

void TCommunicationDeviceWidget::dataReceived(QByteArray data, TReceiverModel * receiverModel)
{
    int modelIndex;
    if ((modelIndex = m_receiverModels.indexOf(receiverModel)) >= 0)
        m_receivedData[modelIndex]->append(data);

    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");

    QString originString;
    if (!receiverModel->name().isEmpty())
        originString = QString(" (%1)").arg(receiverModel->name());

    QString selectedProtocolName = m_receiveProtocolComboBox->currentText();

    if(selectedProtocolName == "raw data") {
        m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received " + QString::number(data.size()) + " B" + originString + "</b>"));
        m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + byteArraytoHumanReadableString(data) + "</div>");
        return;
    }

    bool protocolFound;
    TProtocol selectedReceiveProtocol = m_protocolContainer->getByName(selectedProtocolName, &protocolFound);

    if(!protocolFound) {
        qWarning("Unknown protocol selected, could not interpret message.");
        m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received " + QString::number(data.size()) + " B" + originString + "</b>"));
        m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + byteArraytoHumanReadableString(data) + "</div>");
        return;
    }

    TMessage matchedMessage = selectedReceiveProtocol.tryMatchResponse(data);

    if(matchedMessage.getName().isEmpty()) {
        qWarning("Received data could not be interpreted as any of the protocol's defined messages.");
        m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received " + QString::number(data.size()) + " B" + originString + "</b>"));
        m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + byteArraytoHumanReadableString(data) + "</div>");
        return;
    }

    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Received:" + originString + "</b>"));
    m_communicationLogTextEdit->appendHtml("<div style=\"color:red\">" + matchedMessage.getPayloadSummary() + "</div>");
}

QString TCommunicationDeviceWidget::byteArraytoHumanReadableString(const QByteArray & byteArray)
{
    static QRegularExpression nonAsciiRegExp("[^ -~]");
    bool isHumanReadable = !((QString)byteArray).contains(nonAsciiRegExp);

    if(byteArray.size() <= DISPLAY_DATA_LENGTH_LIMIT) {
        if(!isHumanReadable) {
            return byteArray.toHex(' ');
        }
        else {
            return "<i>\"" + QString(byteArray).toHtmlEscaped() + "\"</i>";
        }
    }
    else {
        if(!isHumanReadable) {
            return QString("%1 ... <i>skipping %2 bytes</i> ... %3")
            .arg(byteArray.first(5).toHex(' '))
                .arg(byteArray.length() - 10)
                .arg(byteArray.last(5).toHex(' '));
        }
        else {
            return QString("<i>\"%1\" ... skipping %2 bytes ... \"%3\"</i>")
            .arg(QString(byteArray.first(5)).toHtmlEscaped())
                .arg(byteArray.length() - 10)
                .arg(QString(byteArray.last(5)).toHtmlEscaped());
        }
    }
}

void TCommunicationDeviceWidget::sendBytes()
{
    if(m_selectedProtocol.getName().isEmpty()) {
        sendRawBytes();
    }
    else {
        sendProtocolBytes();
    }
}

void TCommunicationDeviceWidget::sendRawBytes()
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

    m_currentSenderModel->writeData(dataToWrite);
}

void TCommunicationDeviceWidget::sendProtocolBytes()
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

    m_currentSenderModel->writeData(messageData);
}

void TCommunicationDeviceWidget::sendFile(QString fileName)
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

    m_currentSenderModel->writeData(file.readAll());

    file.close();
}

void TCommunicationDeviceWidget::receiveFile(QString fileName)
{
    int modelIndex;
    if ((modelIndex = m_receiverModels.indexOf(m_currentReceiverModel)) < 0) {
        qWarning("No receive stream selected.");
        return;
    }

    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Data cannot be saved to file because it failed to open.");
        return;
    }

    file.write(*m_receivedData[modelIndex]);

    file.close();
}

void TCommunicationDeviceWidget::sendBusy()
{
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");
    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Failed to send data: device busy</b>"));

    TDialog::deviceFailedBusyMessage(this);
}

void TCommunicationDeviceWidget::sendFailed()
{
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");
    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Failed to send data</b>"));

    TDialog::deviceSendFailedMessage(this);
}

void TCommunicationDeviceWidget::dataSent(QByteArray data, TSenderModel * senderModel)
{
    QTime time = QTime::currentTime();
    QString formattedTime = time.toString("hh:mm:ss");

    QString originString;
    if (!senderModel->name().isEmpty())
        originString = QString(" (%1)").arg(senderModel->name());

    m_communicationLogTextEdit->appendHtml(formattedTime + QString(" <b>Sent " + QString::number(data.size()) + " B" + originString + "</b>"));

    if(data == m_messageToBeSent.getData()) {
        m_communicationLogTextEdit->appendHtml("<div style=\"color:blue\">" + m_messageToBeSent.getPayloadSummary() + "</div>");
    }
    else {
        m_communicationLogTextEdit->appendHtml("<div style=\"color:blue\">" + byteArraytoHumanReadableString(data) + "</div>");
    }
}
