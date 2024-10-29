#include "tanalstreamreceivermodel.h"

TAnalStreamReceiverModel::TAnalStreamReceiverModel(TAnalStreamReceiver * receiver, QObject * parent)
    : TReceiverModel(receiver), m_analReceiver(receiver)
{

}

QString TAnalStreamReceiverModel::name()
{
    return m_analReceiver->name();
}

QString TAnalStreamReceiverModel::info()
{
    return m_analReceiver->info();
}
