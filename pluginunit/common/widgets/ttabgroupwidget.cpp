#include "ttabgroupwidget.h"

#include <QGroupBox>
#include <QLayout>

TTabGroupWidget::TTabGroupWidget(QString groupBoxName, bool tabBarAutohide, QWidget * parent)
    : QGroupBox(groupBoxName, parent)
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabBarAutoHide(tabBarAutohide);

    QLayout * layout = new QGridLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_tabWidget);
    setLayout(layout);
}

void TTabGroupWidget::addWidget(QWidget * widget, QString name, QString description)
{
    m_tabWidget->setTabToolTip(m_tabWidget->addTab(widget, name), description);
}
