#ifndef TPROJECTVIEW_H
#define TPROJECTVIEW_H

#include <QTreeView>

class TProjectView : public QTreeView
{
    Q_OBJECT

public:
    TProjectView(QWidget * parent = nullptr);
};

#endif // TPROJECTVIEW_H
