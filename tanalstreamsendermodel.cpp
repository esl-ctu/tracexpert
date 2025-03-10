#include "tanalstreamsendermodel.h"

TAnalStreamSenderModel::TAnalStreamSenderModel(TAnalStreamSender * sender, QObject * parent)
    : TSenderModel(sender), m_analSender(sender)
{

}

QString TAnalStreamSenderModel::name()
{
    return m_analSender->name();
}

QString TAnalStreamSenderModel::info()
{
    return m_analSender->info();
}
