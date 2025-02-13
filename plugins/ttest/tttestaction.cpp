#include "tttestaction.h"

TTTestAction::TTTestAction(QString name, QString info, std::function<void(void)> run)
    : m_name(name), m_info(info), m_run(run)
{

}

QString TTTestAction::getName() const
{
    return m_name;
}

QString TTTestAction::getInfo() const
{
    return m_info;
}

bool TTTestAction::isEnabled() const
{
    return true;
}

void TTTestAction::run()
{
    m_run();
}

void TTTestAction::abort()
{

}
