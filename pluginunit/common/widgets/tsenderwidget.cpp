#include "tsenderwidget.h"

#include <QGroupBox>

#include "widgets/tfilenameedit.h"
#include "../tdialog.h"

TSenderWidget::TSenderWidget(TSenderModel * senderModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_senderModel(senderModel), m_protocolContainer(protocolContainer)
{
    // Send message ComboBox
    m_messageComboBox = new QComboBox;
    connect(m_messageComboBox, &QComboBox::currentIndexChanged, this, &TSenderWidget::messageChanged);

    m_noMessagesLabel = new QLabel(tr("Protocol doesn't contain any commands."));

    // Raw message row
    m_rawMessageEdit = new QLineEdit();
    connect(m_rawMessageEdit, &QLineEdit::textEdited, this, &TSenderWidget::validateRawInputValues);

    m_rawFormatComboBox = new QComboBox();
    m_rawFormatComboBox->addItem(tr("Hex"));
    m_rawFormatComboBox->addItem(tr("ASCII"));
    connect(m_rawFormatComboBox, &QComboBox::currentIndexChanged, this, &TSenderWidget::validateRawInputValues);

    m_rawMessageEditLayout = new QHBoxLayout();
    m_rawMessageEditLayout->setContentsMargins(0, 0, 0, 0);
    m_rawMessageEditLayout->addWidget(m_rawMessageEdit);
    m_rawMessageEditLayout->addWidget(m_rawFormatComboBox);
    m_rawMessageEditLayout->setStretch(0,4);
    m_rawMessageEditLayout->setStretch(1,1);

    m_protocolComboBox = new QComboBox;
    connect(m_messageComboBox, &QComboBox::currentIndexChanged, this, &TSenderWidget::protocolChanged);

    m_sendButton = new QPushButton("Send");
    connect(m_sendButton, &QPushButton::clicked, this, &TSenderWidget::sendBytes);

    QGroupBox * sendFileGroupBox = new QGroupBox("Send file");

    QLayout * sendFileLayout = new QVBoxLayout();

    TFileNameEdit * sendFileEdit = new TFileNameEdit(QFileDialog::ExistingFile);
    QPushButton * sendFileButton = new QPushButton("Send");
    connect(sendFileButton, &QPushButton::clicked, this, [=](){ sendFile(sendFileEdit->text()); });

    sendFileLayout->addWidget(sendFileEdit);
    sendFileLayout->addWidget(sendFileButton);

    sendFileGroupBox->setLayout(sendFileLayout);

    m_formLayout = new QFormLayout;
    m_formLayout->addRow(tr("Protocol"), m_protocolComboBox);
    m_formLayout->addRow(tr("Message"), m_messageComboBox);
    m_formLayout->setRowVisible(m_messageComboBox, false);
    m_formLayout->addRow(tr("Message"), m_noMessagesLabel);
    m_formLayout->setRowVisible(m_noMessagesLabel, false);
    m_formLayout->addRow(tr("Payload"), m_rawMessageEditLayout);
    m_formLayout->addWidget(m_sendButton);

    m_messageFormManager = new TMessageFormManager(m_formLayout, 3);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addLayout(m_formLayout);
    layout->addWidget(sendFileGroupBox);
    layout->addStretch();

    setLayout(layout);

    updateDisplayedProtocols();
    connect(m_protocolContainer, &TProtocolContainer::protocolsUpdated, this, &TSenderWidget::updateDisplayedProtocols);

    validateRawInputValues();
}

TSenderWidget::~TSenderWidget() {
    delete m_messageFormManager;
}

void TSenderWidget::updateDisplayedProtocols() {
    m_protocolComboBox->clear();

    m_protocolComboBox->addItem("raw data");

    for(int i = 0; i < m_protocolContainer->count(); i++) {
        m_protocolComboBox->addItem(m_protocolContainer->at(i)->name());
    }
}

bool TSenderWidget::validateRawInputValues() {
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

void TSenderWidget::protocolChanged(int index)
{
    m_messageFormManager->clearRows();

    if(index < 1) {
        m_selectedProtocol = TProtocol();

        m_formLayout->setRowVisible(m_rawMessageEditLayout, true);
        m_formLayout->setRowVisible(m_messageComboBox, false);
        m_formLayout->setRowVisible(m_noMessagesLabel, false);
        m_sendButton->setEnabled(true);
        return;
    }

    bool protocolFound;
    m_selectedProtocol = m_protocolContainer->getByName(m_protocolComboBox->currentText(), &protocolFound);

    if(!protocolFound) {
        qWarning("Unknown protocol selected, maybe the user removed it?");
        return;
    }

    m_messageComboBox->clear();
    m_formLayout->setRowVisible(m_rawMessageEditLayout, false);

    for(const TMessage & message : m_selectedProtocol.getMessages()) {
        if(message.isResponse()) {
            continue;
        }

        m_messageComboBox->addItem(message.getName());
    }

    if(m_messageComboBox->count() == 0) {
        m_formLayout->setRowVisible(m_noMessagesLabel, true);
        m_formLayout->setRowVisible(m_messageComboBox, false);
        m_sendButton->setEnabled(false);
        return;
    }

    m_formLayout->setRowVisible(m_noMessagesLabel, false);
    m_formLayout->setRowVisible(m_messageComboBox, true);
    m_sendButton->setEnabled(true);
}

void TSenderWidget::messageChanged(int index)
{
    if(index < 0)
        return;

    if(m_selectedProtocol.getName().isEmpty()) {
        qWarning("No protocol selected, cannot find message!");
        return;
    }

    bool messageFound;
    m_selectedMessage = m_selectedProtocol.getMessageByName(m_messageComboBox->currentText(), &messageFound);

    if(!messageFound) {
        qWarning("Unknown message selected, maybe the user removed it?");
        m_messageFormManager->clearRows();
        return;
    }

    m_messageFormManager->setMessage(m_selectedMessage);
}

void TSenderWidget::sendBytes()
{
    if(m_selectedProtocol.getName().isEmpty()) {
        sendRawBytes();
    }
    else {
        sendProtocolBytes();
    }
}

void TSenderWidget::sendRawBytes()
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

void TSenderWidget::sendProtocolBytes()
{
    if(!m_messageFormManager->assignInputValues()) {
        qWarning("Message could not be sent because user input values were not valid.");
        return;
    }

    TMessage sentMessage = m_messageFormManager->getMessage();
    const QByteArray & messageData = sentMessage.getData();
    if(messageData.length() == 0) {
        qWarning("Message could not be sent because data could not be formed.");
        TDialog::protocolMessageCouldNotBeFormed(this);
        return;
    }

    emit messageSent(sentMessage, m_senderModel);
    m_senderModel->writeData(messageData);
}

void TSenderWidget::sendFile(QString fileName)
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
