#include "tcommunicationdevicewidget.h"

#include <QGroupBox>
#include <QBoxLayout>

#include "ttabgroupwidget.h"
#include "tsenderwidget.h"
#include "treceiverwidget.h"
#include "tcommunicationlogwidget.h"

TCommunicationDeviceWidget::TCommunicationDeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel)
{
    setWindowTitle(tr("IO Device - %1").arg(m_deviceModel->name()));

    m_senderModels.append(deviceModel->senderModel());
    m_receiverModels.append(deviceModel->receiverModel());

    init(protocolContainer);
}

TCommunicationDeviceWidget::TCommunicationDeviceWidget(TAnalDeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWidget(parent), m_deviceModel(deviceModel), m_senderModels(deviceModel->senderModels()), m_receiverModels(deviceModel->receiverModels()), m_actionModels(deviceModel->actionModels())
{
    setWindowTitle(tr("Analytical Device - %1").arg(m_deviceModel->name()));

    init(protocolContainer);
}

void TCommunicationDeviceWidget::init(TProtocolContainer * protocolContainer) {
    setFocusPolicy(Qt::ClickFocus);

    m_paramWidget = new TConfigParamWidget(m_deviceModel->postInitParams());

    QPushButton * applyButton = new QPushButton(tr("Apply"));
    connect(applyButton, &QPushButton::clicked, this, &TCommunicationDeviceWidget::applyPostInitParam);

    QVBoxLayout * paramLayout = new QVBoxLayout;
    paramLayout->addWidget(m_paramWidget);
    paramLayout->addWidget(applyButton);

    QHBoxLayout * textParamLayout = new QHBoxLayout;

    TCommunicationLogWidget * logWidget = new TCommunicationLogWidget(m_senderModels, m_receiverModels);

    textParamLayout->addWidget(logWidget);
    textParamLayout->addLayout(paramLayout);

    QGroupBox * textParamBox = new QGroupBox;
    textParamBox->setLayout(textParamLayout);

    TTabGroupWidget * sendersBox = new TTabGroupWidget("Send data", m_senderModels.isEmpty() || (m_senderModels.length() == 1 && m_senderModels[0]->name().isEmpty()));
    for (int i = 0; i < m_senderModels.length(); i++) {
        TSenderWidget * senderWidget = new TSenderWidget(m_senderModels[i], protocolContainer);
        sendersBox->addWidget(senderWidget, m_senderModels[i]->name(), m_senderModels[i]->info());
        connect(senderWidget, &TSenderWidget::messageSent, logWidget, &TCommunicationLogWidget::messageSent);
    }

    TTabGroupWidget * receiversBox = new TTabGroupWidget("Receive data", m_receiverModels.isEmpty() || (m_receiverModels.length() == 1 && m_receiverModels[0]->name().isEmpty()));
    for (int i = 0; i < m_receiverModels.length(); i++) {
        TReceiverWidget * receiverWidget = new TReceiverWidget(m_receiverModels[i], protocolContainer);
        receiversBox->addWidget(receiverWidget, m_receiverModels[i]->name(), m_receiverModels[i]->info());
        connect(receiverWidget, &TReceiverWidget::protocolSelected, logWidget, &TCommunicationLogWidget::protocolChanged);
    }

    QHBoxLayout * sendReceiveLayout = new QHBoxLayout;
    sendReceiveLayout->addWidget(sendersBox);
    sendReceiveLayout->addWidget(receiversBox);
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

    setLayout(layout);
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
