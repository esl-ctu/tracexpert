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
    connect(protocolView, &QTableView::doubleClicked, this, &TProtocolWidget::onEditButtonClicked);

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

void TProtocolWidget::onRowDoubleClicked(const QModelIndex & index) {
    m_editedItemIndex = index.row();
    openEditor();
}

void TProtocolWidget::onEditButtonClicked() {
    if(m_protocolView->selectionModel()->selectedIndexes().isEmpty()) {
        return;
    }

    m_editedItemIndex = m_protocolView->selectionModel()->selectedIndexes().first().row();
    openEditor();
}

void TProtocolWidget::openEditor(const QString & protocolName, bool *ok) {
    m_editedItemIndex = m_protocolContainer->getIndexByName(protocolName, ok);

    if(m_editedItemIndex < 0) {
        qDebug("Could not find protocol with such name.");
        return;
    }

    openEditor();
}

void TProtocolWidget::openEditor() {
    m_protocolEditor = new TProtocolEditor(m_protocolContainer->at(m_editedItemIndex), m_protocolContainer, this);
    connect(m_protocolEditor, &QWizard::finished, this, &TProtocolWidget::onEditorFinished);
    m_protocolEditor->open();
}

void TProtocolWidget::onEditorFinished(int finished) {
    if(finished != QDialog::Accepted) {
        return;
    }

    if(m_editedItemIndex >= 0) {
        if(!m_protocolContainer->update(m_editedItemIndex, m_protocolEditor->protocol())) {
            qWarning("Failed to update protocol, validation in editor probably failed.");
        }
    }
    else {
        if(!m_protocolContainer->add(m_protocolEditor->protocol())) {
            qWarning("Failed to add protocol, validation in editor probably failed.");
        }
    }
}


void TProtocolWidget::onRemoveButtonClicked() {
    if(m_protocolView->selectionModel()->selectedIndexes().isEmpty())
        return;

    m_protocolContainer->remove(m_protocolView->selectionModel()->selectedIndexes().first().row());
}
