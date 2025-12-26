// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contributors to this file:
// Vojtěch Miškovský (initial author)

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
