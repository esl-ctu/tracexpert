#include "tcpaaction.h"

TCPAAction::TCPAAction(QString name, QString info, std::function<void(void)> run)
    : m_name(name), m_info(info), m_run(run)
{

}

QString TCPAAction::getName() const
{
    return m_name;
}

QString TCPAAction::getInfo() const
{
    return m_info;
}

bool TCPAAction::isEnabled() const
{
    return true;
}

void TCPAAction::run()
{
    m_run();
}

void TCPAAction::abort()
{

}

