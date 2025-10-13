#include "tcipheraction.h"

TCipherAction::TCipherAction(QString name, QString info, std::function<void(void)> run)
    : m_name(name), m_info(info), m_run(run)
{

}

QString TCipherAction::getName() const
{
    return m_name;
}

QString TCipherAction::getInfo() const
{
    return m_info;
}

bool TCipherAction::isEnabled() const
{
    return true;
}

void TCipherAction::run()
{
    m_run();
}

void TCipherAction::abort()
{

}

