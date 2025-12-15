#include "treceiverwidget.h"

#include <QPushButton>
#include <QCheckBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>

#include "../protocol/tprotocolmodel.h"
#include "widgets/tfilenameedit.h"

TReceiverWidget::TReceiverWidget(TReceiverModel * receiverModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_receiverModel(receiverModel), m_protocolContainer(protocolContainer)
{
    m_bytesEdit = new QLineEdit;
    QIntValidator * receiveBytesValidator = new QIntValidator;
    receiveBytesValidator->setBottom(1);
    m_bytesEdit->setValidator(receiveBytesValidator);

    QPushButton * receiveButton = new QPushButton("Receive");
    receiveButton->setEnabled(false);
    connect(receiveButton, &QPushButton::clicked, this, &TReceiverWidget::receiveBytes);
    connect(m_bytesEdit, &QLineEdit::textChanged, this, [=](){receiveButton->setEnabled(m_bytesEdit->hasAcceptableInput());});

    QCheckBox * autoReceiveCheckbox = new QCheckBox;
    autoReceiveCheckbox->setChecked(false);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, this, &TReceiverWidget::setAutoreceive);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, m_bytesEdit, &QLineEdit::setDisabled);
    connect(autoReceiveCheckbox, &QCheckBox::clicked, receiveButton, &QPushButton::setDisabled);

    QLabel * autoReceiveLabel = new QLabel(tr("Autoreceive"));

    QHBoxLayout * autoReceiveLayout = new QHBoxLayout;
    autoReceiveLayout->addStretch();
    autoReceiveLayout->addWidget(autoReceiveLabel);
    autoReceiveLayout->addWidget(autoReceiveCheckbox);

    m_protocolComboBox = new QComboBox;
    connect(m_protocolComboBox, &QComboBox::currentIndexChanged, this, &TReceiverWidget::protocolChanged);

    QFormLayout * formLayout = new QFormLayout;
    formLayout->addRow(tr("Protocol"), m_protocolComboBox);
    formLayout->addRow(tr("Bytes"), m_bytesEdit);
    formLayout->addWidget(receiveButton);

    QGroupBox * receiveFileGroupBox = new QGroupBox("Save current stream to file (raw)");

    QLayout * receiveFileLayout = new QVBoxLayout();

    TFileNameEdit * receiveFileEdit = new TFileNameEdit(QFileDialog::AnyFile);
    QPushButton * receiveFileButton = new QPushButton("Save");
    connect(receiveFileButton, &QPushButton::clicked, this, [=](){ receiveFile(receiveFileEdit->text()); });

    receiveFileLayout->addWidget(receiveFileEdit);
    receiveFileLayout->addWidget(receiveFileButton);

    receiveFileGroupBox->setLayout(receiveFileLayout);

    QVBoxLayout * layout = new QVBoxLayout;
    layout->addLayout(formLayout);
    layout->addLayout(autoReceiveLayout);
    layout->addWidget(receiveFileGroupBox);
    layout->addStretch();

    setLayout(layout);

    updateDisplayedProtocols();
    connect(m_protocolContainer, &TProtocolContainer::itemsUpdated, this, &TReceiverWidget::updateDisplayedProtocols);
}

void TReceiverWidget::updateDisplayedProtocols() {
    m_protocolComboBox->clear();

    m_protocolComboBox->addItem("raw data");

    for(int i = 0; i < m_protocolContainer->count(); i++) {
        m_protocolComboBox->addItem(m_protocolContainer->at(i)->name());
    }
}

void TReceiverWidget::protocolChanged(int index)
{
    TProtocolModel * protocolModel = (TProtocolModel *)m_protocolContainer->getByName(m_protocolComboBox->currentText());

    if(protocolModel)
        emit protocolSelected(*protocolModel->protocol(), m_receiverModel);
}

void TReceiverWidget::setAutoreceive(bool enabled)
{
    if (enabled) {
        m_receiverModel->enableAutoRead();
    }
    else {
        m_receiverModel->disableAutoRead();
    }
}

void TReceiverWidget::receiveBytes()
{
    m_receiverModel->readData(m_bytesEdit->text().toInt());
}

void TReceiverWidget::receiveFile(QString fileName)
{
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Data cannot be saved to file because it failed to open.");
        return;
    }

    file.write(m_receiverModel->receivedData());

    file.close();
}
