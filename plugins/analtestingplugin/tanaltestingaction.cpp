#include "tanaltestingaction.h"

TAnalTestingAction::TAnalTestingAction(QString name, QString info, std::function<void(void)> run)
    : m_name(name), m_info(info), m_run(run)
{

}

QString TAnalTestingAction::getName() const
{
    return m_name;
}

QString TAnalTestingAction::getInfo() const
{
    return m_info;
}

bool TAnalTestingAction::isEnabled() const
{
    return true;
}

void TAnalTestingAction::run()
{
    m_run();
}

void TAnalTestingAction::abort()
{

}
