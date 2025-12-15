#ifndef TCOMMUNICATIONDEVICEWIDGET_H
#define TCOMMUNICATIONDEVICEWIDGET_H

#include "../../protocol/tprotocolcontainer.h"
#include "../../anal/tanaldevicemodel.h"
#include "../../io/tiodevicemodel.h"
#include "../../../common/widgets/tconfigparamwidget.h"

class TCommunicationDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TCommunicationDeviceWidget(TAnalDeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    explicit TCommunicationDeviceWidget(TIODeviceModel * deviceModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);

public slots:
    bool applyPostInitParam();

private:
    void init(TProtocolContainer * protocolContainer);

    TDeviceModel * m_deviceModel;

    QList<TSenderModel *> m_senderModels;
    QList<TReceiverModel *> m_receiverModels;
    QList<TAnalActionModel *> m_actionModels;

    TConfigParamWidget * m_paramWidget;
};

#endif // TCOMMUNICATIONDEVICEWIDGET_H
