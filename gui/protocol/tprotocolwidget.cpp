#include "tprotocolwidget.h"
#include "protocol/tprotocoltableview.h"

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


TProtocolWidget::TProtocolWidget(TProtocolContainer * protocolContainer, QWidget * parent) : QWidget(parent)
{
    setWindowTitle("Protocol manager");

    m_protocolContainer = protocolContainer;

    QPushButton * addMessageButton = new QPushButton("Add");
    connect(addMessageButton, &QPushButton::clicked, this, &TProtocolWidget::onAddButtonClicked);

    QPushButton * editMessageButton = new QPushButton("Edit");
    connect(editMessageButton, &QPushButton::clicked, this, &TProtocolWidget::onEditButtonClicked);

    QPushButton * removeMessageButton = new QPushButton("Remove");
    connect(removeMessageButton, &QPushButton::clicked, this, &TProtocolWidget::onRemoveButtonClicked);

    QVBoxLayout * sideButtonsLayout = new QVBoxLayout();
    sideButtonsLayout->addWidget(addMessageButton);
    sideButtonsLayout->addWidget(editMessageButton);
    sideButtonsLayout->addWidget(removeMessageButton);
    sideButtonsLayout->addStretch();

    QTableView * protocolView = new TProtocolTableView();
    protocolView->setModel(m_protocolContainer);

    m_protocolView = protocolView;

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(protocolView);
    horizontalLayout->addLayout(sideButtonsLayout);

    setLayout(horizontalLayout);
}

void TProtocolWidget::onAddButtonClicked() {
    m_editedItemIndex = -1;

    m_protocolEditor = new TProtocolEditor(TProtocol(), m_protocolContainer, this);
    connect(m_protocolEditor, &QWizard::finished, this, &TProtocolWidget::onEditorFinished);
    m_protocolEditor->open();
}

void TProtocolWidget::onEditButtonClicked() {
    if(m_protocolView->selectionModel()->selectedIndexes().isEmpty())
        return;

    m_editedItemIndex = m_protocolView->selectionModel()->selectedIndexes().first().row();

    m_protocolEditor = new TProtocolEditor(m_protocolContainer->getItem(m_editedItemIndex)->protocol(), m_protocolContainer, this);
    connect(m_protocolEditor, &QWizard::finished, this, &TProtocolWidget::onEditorFinished);
    m_protocolEditor->open();
}

void TProtocolWidget::onEditorFinished(int finished) {
    if(finished != QDialog::Accepted) {
        return;
    }

    if(m_editedItemIndex >= 0) {
        m_protocolContainer->getItem(m_editedItemIndex)->protocol();
        m_protocolContainer->updateItem(m_editedItemIndex, new TProtocolModel(TProtocol(m_protocolEditor->protocol()), m_protocolContainer));
    }
    else {
        m_protocolContainer->insertItem(m_protocolContainer->rowCount(), new TProtocolModel(TProtocol(m_protocolEditor->protocol()), m_protocolContainer));
    }
}


void TProtocolWidget::onRemoveButtonClicked() {
    if(m_protocolView->selectionModel()->selectedIndexes().isEmpty())
        return;

    m_protocolContainer->removeItem(m_protocolView->selectionModel()->selectedIndexes().first().row());
}
