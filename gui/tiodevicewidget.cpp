#include "tiodevicewidget.h"

#include <QLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>

#include "tconfigparamwidget.h"
#include "tdialog.h"

TIODeviceWidget::TIODeviceWidget(TIODeviceModel * deviceModel, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel)
{
    setWindowTitle(tr("IO Device - %1").arg(m_deviceModel->name()));

    QPlainTextEdit * communicationLogTextEdit = new QPlainTextEdit;
    communicationLogTextEdit->setEnabled(false);
    
    m_paramWidget = new TConfigParamWidget(m_deviceModel->postInitParams());

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TIODeviceWidget::applyPostInitParam);

    QVBoxLayout * paramLayout = new QVBoxLayout;
    paramLayout->addWidget(m_paramWidget);
    paramLayout->addWidget(applyButton);

    QHBoxLayout * textParamLayout = new QHBoxLayout;

    textParamLayout->addWidget(communicationLogTextEdit);
    textParamLayout->addLayout(paramLayout);

    QGroupBox * textParamBox = new QGroupBox;
    textParamBox->setLayout(textParamLayout);

    QLineEdit * sendMessageEdit = new QLineEdit;

    QRadioButton * hexRadioButton = new QRadioButton("Hex");
    hexRadioButton->setChecked(true);
    QRadioButton * asciiRadioButton = new QRadioButton("ASCII");

    QLayout * radioLayout = new QVBoxLayout;
    radioLayout->addWidget(hexRadioButton);
    radioLayout->addWidget(asciiRadioButton);

    QGroupBox * radioGroupBox = new QGroupBox("Format");
    radioGroupBox->setLayout(radioLayout);

    QPushButton * sendButton = new QPushButton("Send");

    QHBoxLayout * sendMessageLayout = new QHBoxLayout;
    sendMessageLayout->addWidget(sendMessageEdit);
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

    QHBoxLayout * receiveMessageLayout = new QHBoxLayout;
    receiveMessageLayout->addWidget(receiveBytesLabel);
    receiveMessageLayout->addWidget(m_receiveBytesEdit);
    receiveMessageLayout->addWidget(receiveButton);

    QGroupBox * receiveMessageBox = new QGroupBox("Receive data");
    receiveMessageBox->setLayout(receiveMessageLayout);

    QHBoxLayout * sendReceiveLayout = new QHBoxLayout;
    sendReceiveLayout->addWidget(sendMessageBox);
    sendReceiveLayout->addWidget(receiveMessageBox);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addWidget(textParamBox);
    layout->addLayout(sendReceiveLayout);

    setLayout(layout);
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

void TIODeviceWidget::receiveBytes()
{
    m_deviceModel->readData(m_receiveBytesEdit->text().toInt());
}

void TIODeviceWidget::receiveBusy()
{
    TDialog::deviceFailedBusyMessage(this);
}
