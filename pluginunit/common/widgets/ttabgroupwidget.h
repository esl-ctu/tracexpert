#ifndef TCOMMUNICATIONSTREAMSWIDGET_H
#define TCOMMUNICATIONSTREAMSWIDGET_H

#include <QGroupBox>
#include <QTabWidget>

class TTabGroupWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit TTabGroupWidget(QString groupBoxName, bool tabBarAutohide, QWidget * parent = nullptr);

    void addWidget(QWidget * widget, QString name, QString description = QString());

private:
    QTabWidget * m_tabWidget;
};

#endif // TCOMMUNICATIONSTREAMSWIDGET_H
