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
// Adam Švehla (initial author)

#ifndef TPROJECTUNITWIDGET_H
#define TPROJECTUNITWIDGET_H

#include <QWidget>

#include "tprojectunitcontainer.h"
#include "tprojectunittableview.h"

/*!
 * \brief The TProjectUnitWidget class represents a widget for managing project units, such as Protocols or Scenarios.
 *
 * The class represents a widget for managing project units, such as Protocols or Scenarios.
 * It allows the user to add, edit, rename and remove project units through the GUI.
 *
 */
class TProjectUnitWidget : public QWidget {
    Q_OBJECT

public:
    explicit TProjectUnitWidget(TProjectUnitContainer * protocolContainer, QWidget * parent = nullptr);

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onRowDoubleClicked(const QModelIndex & index);
    void onRemoveButtonClicked();
    void onRenameButtonClicked();

    void onDuplicateButtonClicked();

    void onImportButtonClicked();
    void onExportButtonClicked();

protected:
    uint importFromFiles();
    bool exportToFiles(const TProjectUnitModel * model);

    QString m_fileTypeFilter;

    TProjectUnitTableView * m_tableView;
    TProjectUnitContainer * m_container;
};

#endif // TPROJECTUNITWIDGET_H
