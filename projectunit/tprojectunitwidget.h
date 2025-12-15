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
