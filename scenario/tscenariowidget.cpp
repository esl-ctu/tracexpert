#include "tscenariowidget.h"
#include "../protocol/tprotocoltableview.h"

#include <QListWidget>
#include <QLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QTableView>
#include <QDialog>
#include <QHeaderView>

#include "../tdialog.h"
#include "tscenario.h"

TScenarioWidget::TScenarioWidget(TScenarioContainer * scenarioContainer, QWidget * parent) :
    QWidget(parent), m_mainWindow((TMainWindow *)parent), m_scenarioContainer(scenarioContainer)
{
    QPushButton * addMessageButton = new QPushButton("Add");
    connect(addMessageButton, &QPushButton::clicked, this, &TScenarioWidget::onAddButtonClicked);

    QPushButton * editMessageButton = new QPushButton("Edit");
    connect(editMessageButton, &QPushButton::clicked, this, &TScenarioWidget::onEditButtonClicked);

    QPushButton * renameMessageButton = new QPushButton("Rename");
    connect(renameMessageButton, &QPushButton::clicked, this, &TScenarioWidget::onRenameButtonClicked);

    QPushButton * removeMessageButton = new QPushButton("Remove");
    connect(removeMessageButton, &QPushButton::clicked, this, &TScenarioWidget::onRemoveButtonClicked);

    QVBoxLayout * sideButtonsLayout = new QVBoxLayout();
    sideButtonsLayout->addWidget(addMessageButton);
    sideButtonsLayout->addWidget(editMessageButton);
    sideButtonsLayout->addWidget(renameMessageButton);
    sideButtonsLayout->addWidget(removeMessageButton);
    sideButtonsLayout->addStretch();

    m_scenarioView = new TProtocolTableView();
    m_scenarioView->setModel(m_scenarioContainer);
    connect(m_scenarioView, &QTableView::doubleClicked, this, &TScenarioWidget::onEditButtonClicked);

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(m_scenarioView);
    horizontalLayout->addLayout(sideButtonsLayout);

    setLayout(horizontalLayout);
}

void TScenarioWidget::onAddButtonClicked() {
    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info))
        return;

    TScenario * scenario = new TScenario(name, info);
    bool ok = m_scenarioContainer->add(scenario);
    if (!ok) {
        delete scenario;
        TDialog::parameterValueNotUniqueMessage(this, "name");
    }
}

void TScenarioWidget::onRenameButtonClicked() {
    if(m_scenarioView->selectionModel()->selectedIndexes().isEmpty())
        return;

    int idx = m_scenarioView->selectionModel()->selectedIndexes().first().row();
    TScenario * selectedScenario = m_scenarioContainer->at(idx)->scenario();

    QString name = selectedScenario->getName();
    QString info = selectedScenario->getDescription();

    if (!TDialog::renameDeviceDialog(this, name, info))
        return;

    selectedScenario->setName(name);
    selectedScenario->setDescription(info);
}


void TScenarioWidget::onRowDoubleClicked(const QModelIndex & index) {
    int idx = index.row();
    openEditor(m_scenarioContainer->at(idx));
}

void TScenarioWidget::onEditButtonClicked() {
    if(m_scenarioView->selectionModel()->selectedIndexes().isEmpty())
        return;

    int idx = m_scenarioView->selectionModel()->selectedIndexes().first().row();
    openEditor(m_scenarioContainer->at(idx));
}

void TScenarioWidget::openEditor(TScenarioModel * scenario) {
    m_mainWindow->openScenarioEditor(scenario);
}

void TScenarioWidget::onRemoveButtonClicked() {
    if(m_scenarioView->selectionModel()->selectedIndexes().isEmpty())
        return;

    m_scenarioContainer->remove(m_scenarioView->selectionModel()->selectedIndexes().first().row());
}
