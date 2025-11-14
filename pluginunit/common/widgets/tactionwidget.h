#ifndef TACTIONWIDGET_H
#define TACTIONWIDGET_H

#include <QPushButton>

#include "../../anal/action/tanalactionmodel.h"

class TActionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TActionWidget(TAnalActionModel * actionModel, QWidget * parent = nullptr);

public slots:
    void actionStarted(TAnalActionModel * actionModel);
    void actionFinished();

private slots:
    void runAction();
    void abortAction();

private:
    TAnalActionModel * m_actionModel;

    QPushButton * m_runButton;
    QPushButton * m_abortButton;
};

#endif // TACTIONWIDGET_H
