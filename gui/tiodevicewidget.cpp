#include "tiodevicewidget.h"

#include <QLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QCoreApplication>

#include "qcheckbox.h"
#include "tconfigparamwidget.h"
#include "tdialog.h"

TIODeviceWidget::TIODeviceWidget(TIODeviceModel * deviceModel, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel)
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

    QPushButton * sendButton = new QPushButton("Send");
    connect(sendButton, &QPushButton::clicked, this, &TIODeviceWidget::sendBytes);

    QHBoxLayout * sendMessageLayout = new QHBoxLayout;
    sendMessageLayout->addWidget(m_sendMessageEdit);
    sendMessageLayout->addWidget(radioGroupBox);
    sendMessageLayout->addWidget(sendButton);

    QGroupBox * sendMessageBox = new QGroupBox("Send data");
    sendMessageBox->setLayout(sendMessageLayout);

    QLabel * receiveBytesLabel = new QLabel("Bytes");
    m_receiveBytesEdit = new QLineEdit;
    QIntValidator * receiveBytesValidator = new QIntValidator;
    receiveBytesValidator->setBottom(1);
    m_receiveBytesEdit->setValidator(receiveBytesValidator);

    QPushButton * receiveButton = new QPushButton("Receive");
    receiveButton->setEnabled(false);
    connect(receiveButton, &QPushButton::clicked, this, &TIODeviceWidget::receiveBytes);

    connect(m_receiveBytesEdit, &QLineEdit::textChanged, this, [=](){receiveButton->setEnabled(m_receiveBytesEdit->hasAcceptableInput());});

    connect(m_deviceModel, &TIODeviceModel::readBusy, this, &TIODeviceWidget::receiveBusy);

    QCheckBox * autoReceiveCheckbox = new QCheckBox("Autoreceive");
    autoReceiveCheckbox->setChecked(false);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, this, &TIODeviceWidget::setAutoreceive);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, m_receiveBytesEdit, &QLineEdit::setDisabled);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, receiveButton, &QPushButton::setDisabled);

    QHBoxLayout * receiveMessageLayout = new QHBoxLayout;
    receiveMessageLayout->addWidget(receiveBytesLabel);
    receiveMessageLayout->addWidget(m_receiveBytesEdit);
    receiveMessageLayout->addWidget(receiveButton);

    QVBoxLayout * receiveLayout = new QVBoxLayout;
    receiveLayout->addWidget(autoReceiveCheckbox);
    receiveLayout->addLayout(receiveMessageLayout);

    QGroupBox * receiveMessageBox = new QGroupBox("Receive data");
    receiveMessageBox->setLayout(receiveLayout);

    QHBoxLayout * sendReceiveLayout = new QHBoxLayout;
    sendReceiveLayout->addWidget(sendMessageBox);
    sendReceiveLayout->addWidget(receiveMessageBox);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(textParamBox);
    layout->addLayout(sendReceiveLayout);

    setLayout(layout);

    connect(m_deviceModel, &TIODeviceModel::readFailed, this, &TIODeviceWidget::receiveFailed);
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
    m_communicationLogTextEdit->appendPlainText(data);
    QCoreApplication::processEvents(QEventLoop::ProcessEventsFlag::AllEvents);
}

void TIODeviceWidget::sendBytes()
{
    m_deviceModel->writeData(m_sendMessageEdit->text().toUtf8());
    m_communicationLogTextEdit->appendPlainText(m_sendMessageEdit->text());
}

void TIODeviceWidget::sendBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}

void TIODeviceWidget::sendFailed()
{
    TDialog::deviceSendFailedMessage(this);
}
