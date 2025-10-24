#include "tpluginunitcontainerview.h"

#include <QHeaderView>

TPluginUnitContainerView::TPluginUnitContainerView(QWidget * parent)
    : QTableView(parent)
{
    setSelectionBehavior(QTableView::SelectionBehavior::SelectRows);
    setSelectionMode(QTableView::SelectionMode::SingleSelection);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
    setShowGrid(false);

    verticalHeader()->hide();
    verticalHeader()->setHighlightSections(false);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    horizontalHeader()->setHighlightSections(false);
    horizontalHeader()->setStretchLastSection(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    //horizontalHeader()->setStretchLastSection(false);
    //resizeRowsToContents();
    //resizeColumnsToContents();
    //horizontalHeader()->setStretchLastSection(true);
}
