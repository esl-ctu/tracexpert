#include "tpredictaction.h"

TPredictAction::TPredictAction(QString name, QString info, std::function<void(void)> run)
    : m_name(name), m_info(info), m_run(run)
{

}

QString TPredictAction::getName() const
{
    return m_name;
}

QString TPredictAction::getInfo() const
{
    return m_info;
}

bool TPredictAction::isEnabled() const
{
    return true;
}

void TPredictAction::run()
{
    m_run();
}

void TPredictAction::abort()
{

}

