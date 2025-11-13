#include "tcommunicationdevicewidget.h"

#include <QGroupBox>
#include <QBoxLayout>

#include "ttabgroupwidget.h"
#include "tsenderwidget.h"
#include "treceiverwidget.h"
#include "tactionwidget.h"
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

    if (!m_actionModels.isEmpty()) {
        TTabGroupWidget * actionsBox = new TTabGroupWidget("Actions", false);
        for (int i = 0; i < m_actionModels.length(); i++) {
            TActionWidget * actionWidget = new TActionWidget(m_actionModels[i]);
            actionsBox->addWidget(actionWidget, m_actionModels[i]->name(), m_actionModels[i]->info());
            for (int j = 0; j < m_actionModels.length(); j++) {
                connect(m_actionModels[j], &TAnalActionModel::started, actionWidget, [=](){ actionWidget->actionStarted(m_actionModels[j]); });
                connect(m_actionModels[j], &TAnalActionModel::finished, actionWidget, &TActionWidget::actionFinished);
            }
        }
        layout->addWidget(actionsBox);
    }

    setLayout(layout);
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
