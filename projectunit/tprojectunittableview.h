#ifndef TPROJECTUNITTABLEVIEW_H
#define TPROJECTUNITTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QHeaderView>

class TProjectUnitTableView : public QTableView
{
public:
    TProjectUnitTableView(QWidget * parent = nullptr) : QTableView(parent)
    {
        setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
        setSelectionMode(QTableView::SelectionMode::ExtendedSelection);
        setFocusPolicy(Qt::FocusPolicy::NoFocus);

        verticalHeader()->hide();

        horizontalHeader()->setHighlightSections(false);
        horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    }
};

#endif // TPROJECTUNITTABLEVIEW_H
