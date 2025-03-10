#include "tanalactionaborter.h"

TAnalActionAborter::TAnalActionAborter(TAnalAction * action, QObject * parent)
    : QObject(parent), m_action(action)
{

}

void TAnalActionAborter::abort()
{
    m_action->abort();
}
