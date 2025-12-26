#include "tprojectunitwidget.h"

#include <QLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QToolBar>

#include "../tdialog.h"
#include "tprojectunit.h"

TProjectUnitWidget::TProjectUnitWidget(TProjectUnitContainer * container, QWidget * parent) :
    QWidget(parent), m_container(container)
{
    QAction * addAction = new QAction(tr("&Add"), this);
    addAction->setStatusTip(tr("Create a new %1").arg(container->unitTypeName()));
    addAction->setIcon(QPixmap(":/icons/add.png"));
    connect(addAction, &QAction::triggered, this, &TProjectUnitWidget::onAddButtonClicked);

    QAction * editAction = new QAction(tr("&Edit"), this);
    editAction->setStatusTip(tr("Edit selected %1").arg(container->unitTypeName()));
    editAction->setIcon(QPixmap(":/icons/show.png"));
    connect(editAction, &QAction::triggered, this, &TProjectUnitWidget::onEditButtonClicked);

    QAction * renameAction = new QAction(tr("Re&name"), this);
    renameAction->setStatusTip(tr("Rename selected %1").arg(container->unitTypeName()));
    renameAction->setIcon(QPixmap(":/icons/rename.png"));
    connect(renameAction, &QAction::triggered, this, &TProjectUnitWidget::onRenameButtonClicked);

    QAction * removeAction = new QAction(tr("&Remove"), this);
    removeAction->setStatusTip(tr("Remove selected %1").arg(container->unitTypeName()));
    removeAction->setIcon(QPixmap(":/icons/delete.png"));
    connect(removeAction, &QAction::triggered, this, &TProjectUnitWidget::onRemoveButtonClicked);

    QAction * duplicateAction = new QAction(tr("&Duplicate"), this);
    duplicateAction->setStatusTip(tr("Duplicate selected %1").arg(container->unitTypeName()));
    duplicateAction->setIcon(QPixmap(":/icons/duplicate.png"));
    connect(duplicateAction, &QAction::triggered, this, &TProjectUnitWidget::onDuplicateButtonClicked);

    QAction * importAction = new QAction(tr("&Import"), this);
    importAction->setStatusTip(tr("Import %1(s) from file(s)").arg(container->unitTypeName()));
    importAction->setIcon(QPixmap(":/icons/import.png"));
    connect(importAction, &QAction::triggered, this, &TProjectUnitWidget::onImportButtonClicked);

    QAction * exportAction = new QAction(tr("E&xport"), this);
    exportAction->setStatusTip(tr("Export %1(s) to file(s)").arg(container->unitTypeName()));
    exportAction->setIcon(QPixmap(":/icons/export.png"));
    connect(exportAction, &QAction::triggered, this, &TProjectUnitWidget::onExportButtonClicked);

    QToolBar * toolBar = new QToolBar();
    toolBar->addAction(editAction);
    toolBar->addSeparator();
    toolBar->addAction(addAction);    
    toolBar->addAction(renameAction);
    toolBar->addAction(duplicateAction);
    toolBar->addAction(removeAction);
    toolBar->addSeparator();
    toolBar->addAction(importAction);
    toolBar->addAction(exportAction);

    QHBoxLayout * toolbarLayout = new QHBoxLayout;
    toolbarLayout->addWidget(toolBar);

    m_tableView = new TProjectUnitTableView();
    m_tableView->setModel(m_container);
    connect(m_tableView, &QTableView::doubleClicked, this, &TProjectUnitWidget::onEditButtonClicked);

    QVBoxLayout * layout = new QVBoxLayout();
    layout->addLayout(toolbarLayout);
    layout->addWidget(m_tableView);

    setLayout(layout);
}

void TProjectUnitWidget::onAddButtonClicked() {
    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info))
        return;

    TProjectUnit * unit = TProjectUnit::instantiate(m_container->unitTypeName());
    unit->setName(name);
    unit->setDescription(info);

    bool ok = m_container->add(unit);
    if (!ok) {
        delete unit;
        TDialog::parameterValueNotUniqueMessage(this, "name");
    }
}

void TProjectUnitWidget::onRenameButtonClicked() {
    if(m_tableView->selectionModel()->selectedIndexes().isEmpty())
        return;

    int idx = m_tableView->selectionModel()->selectedIndexes().first().row();
    TProjectUnit * selectedUnit = m_container->at(idx)->unit();

    QString name = selectedUnit->name();
    QString info = selectedUnit->description();

    if (!TDialog::renameDeviceDialog(this, name, info))
        return;

    selectedUnit->setName(name);
    selectedUnit->setDescription(info);
}


void TProjectUnitWidget::onRowDoubleClicked(const QModelIndex & index) {
    int idx = index.row();
    m_container->at(idx)->openEditor();
}

void TProjectUnitWidget::onEditButtonClicked() {
    if(m_tableView->selectionModel()->selectedIndexes().isEmpty())
        return;

    int idx = m_tableView->selectionModel()->selectedIndexes().first().row();
    m_container->at(idx)->openEditor();
}

void TProjectUnitWidget::onRemoveButtonClicked() {
    QStringList unitNames;

    QModelIndexList rows = m_tableView->selectionModel()->selectedRows();
    if (rows.isEmpty())
        return;
    for (const QModelIndex &rowIndex : rows) {
        unitNames << m_container->at(rowIndex.row())->name();
    }

    if (!TDialog::itemsRemoveQuestion(this, unitNames.join("\n"))) {
        return;
    }

    for (const QString & unitName : unitNames) {
        m_container->remove(m_container->getIndexByName(unitName));
    }
}

void TProjectUnitWidget::onDuplicateButtonClicked() {
    if(m_tableView->selectionModel()->selectedIndexes().isEmpty())
        return;

    int selectedIndex = m_tableView->selectionModel()->selectedIndexes().first().row();

    TProjectUnit * unit = m_container->at(selectedIndex)->unit();

    QByteArray array;
    QDataStream stream(&array, QIODeviceBase::ReadWrite);
    unit->serialize(stream);

    // reset the device to the beginning
    stream.device()->seek(0);

    TProjectUnit * unitCopy = TProjectUnit::deserialize(m_container->unitTypeName(), stream);
    if(!unitCopy) {
        qWarning("Failed to duplicate %s.", qPrintable(m_container->unitTypeName()));
        return;
    }

    unitCopy->setName(tr("Copy of ") + unit->name());

    uint i = 1;
    while(!m_container->add(unitCopy)) {
        unitCopy->setName(tr("Copy of %1 (%2)").arg(unit->name()).arg(i++));
    }
}

void TProjectUnitWidget::onImportButtonClicked() {
    try {
        uint loadedCount = importFromFiles();
        if(loadedCount > 0) {
            QMessageBox::information(this, tr("Import successful"), tr("Imported %1 %2(s) successfully.").arg(loadedCount).arg(m_container->unitTypeName()));
        }
    }
    catch(QString message) {
        QMessageBox::critical(this, tr("Import failed"), tr("Unable to import file: %1").arg(message));
    }
}

void TProjectUnitWidget::onExportButtonClicked() {
    if(m_tableView->selectionModel()->selectedIndexes().isEmpty()) {
        QMessageBox::warning(this, tr("Export failed"), tr("Select %1(s) to be exported!").arg(m_container->unitTypeName()));
        return;
    }

    QModelIndexList rows = m_tableView->selectionModel()->selectedRows();

    int savedCount = 0;
    for (const QModelIndex &rowIndex : rows) {
        const TProjectUnitModel * unitModel = m_container->at(rowIndex.row());

        try {
            if(exportToFiles(unitModel)) {
                savedCount++;
            }
        }
        catch(QString message) {
            QMessageBox::critical(this, tr("Export failed"), tr("Unable to export selected %1 file: %2").arg(m_container->unitTypeName(), message));
            break;
        }
    }

    if(savedCount > 0) {
        QMessageBox::information(this, tr("Export successful"), tr("Exported %1 %2(s) successfully.").arg(savedCount).arg(m_container->unitTypeName()));
    }
}

uint TProjectUnitWidget::importFromFiles() {
    QStringList filters;
    filters << m_fileTypeFilter
            << "Any files (*)";

    QFileDialog openDialog;
    openDialog.setNameFilters(filters);
    openDialog.setAcceptMode(QFileDialog::AcceptOpen);
    openDialog.setFileMode(QFileDialog::ExistingFiles);

    if (!openDialog.exec()) return 0;

    QStringList files = openDialog.selectedFiles();

    for (const QString &fileName : files) {
        QFile protocolFile(fileName);
        protocolFile.open(QIODevice::ReadOnly | QIODevice::Text);

        QByteArray documentArray = protocolFile.readAll();
        protocolFile.close();

        QDomDocument document;
        document.setContent(documentArray);

        QDomElement projectElement = document.documentElement();
        TProjectUnitModel * model = TProjectUnitModel::instantiate(m_container->unitTypeName(), m_container);

        try {
            model->load(&projectElement);
            if(!m_container->add(model)) {
                throw tr("A scenario with the name \"%1\" already exists!").arg(model->name());
            }
        }
        catch (QString message) {
            if (model) {
                delete model;
            }
            throw message;
        }
    }

    return files.count();
}

bool TProjectUnitWidget::exportToFiles(const TProjectUnitModel * model) {
    QDomDocument document;
    document.appendChild(model->save(document));

    QStringList filters;
    filters << m_fileTypeFilter
            << "Any files (*)";

    QFileDialog saveDialog;
    saveDialog.setNameFilters(filters);
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setFileMode(QFileDialog::AnyFile);

    saveDialog.selectFile(model->name());
    if (!saveDialog.exec()) return false;

    QFile projectFile(saveDialog.selectedFiles().constFirst());
    projectFile.open(QIODevice::WriteOnly | QIODevice::Text);
    projectFile.write(document.toByteArray());
    projectFile.close();

    return true;
}
