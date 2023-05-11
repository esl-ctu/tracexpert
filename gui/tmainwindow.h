#ifndef TMAINWINDOW_H
#define TMAINWINDOW_H

#include <QMainWindow>
#include "DockManager.h"

class TMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    TMainWindow(QWidget *parent = nullptr);
    ~TMainWindow();

private:
    ads::CDockManager* m_DockManager;
};
#endif // TMAINWINDOW_H
