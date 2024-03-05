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
        verticalHeader()->setHighlightSections(false);
        verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

        horizontalHeader()->setHighlightSections(false);
        horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    }

};

#endif // TPROTOCOLTABLEVIEW_H
