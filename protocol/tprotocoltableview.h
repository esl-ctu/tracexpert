#ifndef TPROTOCOLTABLEVIEW_H
#define TPROTOCOLTABLEVIEW_H

#include <QWidget>
#include <QTableView>
#include <QHeaderView>

class TProtocolTableView : public QTableView
{
public:
    TProtocolTableView(QWidget * parent = nullptr) : QTableView(parent)
    {
        setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
        setSelectionMode(QTableView::SelectionMode::SingleSelection);
        setFocusPolicy(Qt::FocusPolicy::NoFocus);

        verticalHeader()->hide();

        horizontalHeader()->setHighlightSections(false);
        horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
    }
};

#endif // TPROTOCOLTABLEVIEW_H
