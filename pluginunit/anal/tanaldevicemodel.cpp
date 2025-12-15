#include "tanaldevicemodel.h"

#include "../component/tcomponentmodel.h"
#include "stream/tanalstreamsender.h"
#include "stream/tanalstreamreceiver.h"

TAnalDeviceModel::TAnalDeviceModel(TAnalDevice * analDevice, TAnalDeviceContainer * parent, bool manual)
    : TProjectItem(parent->model(), parent), TDeviceModel(analDevice, parent, manual), m_analDevice(analDevice)
{
    m_typeName = "analdevice";
}

TAnalDeviceModel::~TAnalDeviceModel() {
    if (!isInit())
        return;

    TAnalDeviceModel::deInit();
}

void TAnalDeviceModel::show()
{
    emit showRequested(this);
}

bool TAnalDeviceModel::init()
{
    if (isInit() || !TPluginUnitModel::init()) {
        return false;
    }

    m_isInit = true;

    QList<TAnalInputStream *> inputStreams = m_analDevice->getInputDataStreams();
    QList<TAnalOutputStream *> outputStreams = m_analDevice->getOutputDataStreams();
    QList<TAnalAction *> actions = m_analDevice->getActions();

    for (int i = 0; i < outputStreams.length(); i++) {
        m_senderModels.append(new TSenderModel(new TAnalStreamSender(outputStreams[i])));
    }

    for (int i = 0; i < inputStreams.length(); i++) {
        m_receiverModels.append(new TReceiverModel(new TAnalStreamReceiver(inputStreams[i])));
    }

    for (int i = 0; i < actions.length(); i++) {
        m_actionModels.append(new TAnalActionModel(actions[i]));
    }

    emit initialized(this);
    itemDataChanged();

    return true;
}

bool TAnalDeviceModel::deInit()
{
    for (int i = 0; i < m_receiverModels.length(); i++) {
        m_receiverModels[i]->disableAutoRead();
    }

    if (!isInit() || !TPluginUnitModel::deInit()) {
        return false;
    }

    m_isInit = false;

    for (int i = 0; i < m_senderModels.length(); i++) {
        delete m_senderModels[i];
    }
    m_senderModels.clear();

    for (int i = 0; i < m_receiverModels.length(); i++) {
        delete m_receiverModels[i];
    }
    m_receiverModels.clear();

    for (int i = 0; i < m_actionModels.length(); i++) {
        delete m_actionModels[i];
    }
    m_actionModels.clear();

    emit deinitialized(this);
    itemDataChanged();

    return true;
}

bool TAnalDeviceModel::remove()
{
    TComponentModel * component = dynamic_cast<TComponentModel *>(TProjectItem::parent()->parent());
    if (!component)
        return false;

    return component->removeAnalDevice(this);
}

void TAnalDeviceModel::bind(TCommon * unit)
{
    m_analDevice = static_cast<TAnalDevice *>(unit);
    TPluginUnitModel::bind(m_analDevice);
}

void TAnalDeviceModel::release()
{
    m_analDevice = nullptr;
    TPluginUnitModel::release();
}

QList<TSenderModel *> TAnalDeviceModel::senderModels()
{
    return m_senderModels;
}

QList<TReceiverModel *> TAnalDeviceModel::receiverModels()
{
    return m_receiverModels;
}

QList<TAnalActionModel *> TAnalDeviceModel::actionModels()
{
    return m_actionModels;
}
