#include "tanalactionrunner.h"

TAnalActionRunner::TAnalActionRunner(TAnalAction * action, QObject * parent)
    : QObject(parent), m_action(action)
{

}

void TAnalActionRunner::run()
{
    if (!m_action->isEnabled())
        return;

    emit started();

    m_action->run();

    emit finished();
}
