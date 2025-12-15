#include "tactionwidget.h"

#include <QLayout>
#include <QLabel>
#include <QComboBox>

TActionWidget::TActionWidget(TAnalActionModel * actionModel, QWidget * parent)
    : QWidget(parent), m_actionModel(actionModel)
{
    m_runButton = new QPushButton(tr("Run"));
    m_runButton->setEnabled(m_actionModel->isEnabled());
    connect(m_runButton, &QPushButton::clicked, this, &TActionWidget::runAction);

    m_abortButton = new QPushButton(tr("Abort"));
    m_abortButton->setEnabled(false);
    connect(m_abortButton, &QPushButton::clicked, this, &TActionWidget::abortAction);

    QHBoxLayout * layout = new QHBoxLayout;
    layout->addWidget(m_runButton);
    layout->addWidget(m_abortButton);

    setLayout(layout);
}

void TActionWidget::actionStarted(TAnalActionModel * actionModel)
{
    m_runButton->setEnabled(false);
    if (actionModel == m_actionModel)
        m_abortButton->setEnabled(true);
}

void TActionWidget::actionFinished()
{
    m_runButton->setEnabled(m_actionModel->isEnabled());
    m_abortButton->setEnabled(false);
}

void TActionWidget::runAction()
{
    emit m_actionModel->run();
}

void TActionWidget::abortAction()
{
    emit m_actionModel->abort();
}
