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

#include <QListWidget>
#include <QLayout>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QPushButton>
#include <QListView>
#include <QFormLayout>
#include <QHeaderView>

#include "tmessageeditor.h"
#include "tmessageparteditor.h"
#include "../projectunit/tprojectunittableview.h"
#include "../tdialog.h"

TMessageEditorDetailsPage::TMessageEditorDetailsPage(const TMessage & message, const QList<TMessage> & messageList, QWidget * parent)
    : QWizardPage(parent), m_originalName(message.getName()), m_messageList(messageList) {
    setTitle("Message details");
    setSubTitle("Set message name, description and type");

    m_nameLineEdit = new QLineEdit(message.getName());

    QLineEdit * descLineEdit = new QLineEdit(message.getDescription());

    QComboBox * typeComboBox = new QComboBox();
    typeComboBox->addItem("Response", true);
    typeComboBox->addItem("Command", false);
    typeComboBox->setCurrentIndex(message.isResponse() ? 0 : 1);

    QFormLayout * formLayout = new QFormLayout();
    formLayout->addRow(tr("&Name:"), m_nameLineEdit);
    formLayout->addRow(tr("&Description:"), descLineEdit);
    formLayout->addRow(tr("&Type:"), typeComboBox);

    registerField("name", m_nameLineEdit);
    registerField("description", descLineEdit);
    registerField("isResponse", typeComboBox, "currentData", "currentIndexChanged");

    setLayout(formLayout);
}

bool TMessageEditorDetailsPage::validatePage() {
    if(field("name").toString().isEmpty()) {
        TDialog::parameterValueEmpty(this, tr("name"));
        return false;
    }

    // if name was changed, check uniqueness
    bool isUnique = true;
    if(field("name").toString() != m_originalName) {
        for(const TMessage & message : m_messageList) {
            if(field("name").toString() == message.getName()) {
                isUnique = false;
                break;
            }
        }
    }

    if(!isUnique) {
        TDialog::parameterValueNotUniqueMessage(this, tr("name"));
        return false;
    }

    return true;
}

TMessageEditor::TMessageEditor(const TMessage & message, const QList<TMessage> & messageList, QWidget * parent) : QWizard(parent), m_message(message) {

    setWindowTitle("Message wizard");
    setOption(QWizard::CancelButtonOnLeft);
    setWizardStyle(QWizard::ModernStyle);

    QWizardPage * messagePartListPage = new QWizardPage;
    messagePartListPage->setTitle("Message parts");
    messagePartListPage->setSubTitle("Define message parts");


    QPushButton * addButton = new QPushButton("Add");
    connect(addButton, &QPushButton::clicked, this, &TMessageEditor::onAddButtonClicked);

    QPushButton * editButton = new QPushButton("Edit");
    connect(editButton, &QPushButton::clicked, this, &TMessageEditor::onEditButtonClicked);

    QPushButton * removeButton = new QPushButton("Remove");
    connect(removeButton, &QPushButton::clicked, this, &TMessageEditor::onRemoveButtonClicked);

    QPushButton * moveUpButton = new QPushButton("Move up");
    connect(moveUpButton, &QPushButton::clicked, this, &TMessageEditor::onMoveUpButtonClicked);

    QPushButton * moveDownButton = new QPushButton("Move down");
    connect(moveDownButton, &QPushButton::clicked, this, &TMessageEditor::onMoveDownButtonClicked);

    QVBoxLayout * sideButtonsLayout = new QVBoxLayout();
    sideButtonsLayout->addWidget(addButton);
    sideButtonsLayout->addWidget(editButton);
    sideButtonsLayout->addWidget(removeButton);
    sideButtonsLayout->addSpacing(10);
    sideButtonsLayout->addWidget(moveUpButton);
    sideButtonsLayout->addWidget(moveDownButton);
    sideButtonsLayout->addStretch();

    QTableView * messagePartView = new TProjectUnitTableView();
    m_messagePartContainer = new TMessagePartContainer(message.getMessageParts());
    messagePartView->setModel(m_messagePartContainer);
    messagePartView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
    connect(messagePartView, &QTableView::doubleClicked, this, &TMessageEditor::onEditButtonClicked);

    m_messagePartView = messagePartView;

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(messagePartView);
    horizontalLayout->addLayout(sideButtonsLayout);

    messagePartListPage->setLayout(horizontalLayout);

    addPage(new TMessageEditorDetailsPage(message, messageList, this));

    addPage(messagePartListPage);
}

TMessage TMessageEditor::message() {
    TMessage constructedMessage(
        field("name").toString(),
        field("description").toString(),
        field("isResponse").toBool()
    );

    int size = m_messagePartContainer->rowCount();
    for(int i = 0; i < size; i++) {
        constructedMessage.addMessagePart(m_messagePartContainer->getItem(i));
    }

    constructedMessage.validateMessage();

    return constructedMessage;
}

void TMessageEditor::onAddButtonClicked() {
    m_editedItemIndex = -1;

    m_addedItemIndex = -1;
    if(!m_messagePartView->selectionModel()->selectedIndexes().isEmpty()) {
        m_addedItemIndex = m_messagePartView->selectionModel()->selectedIndexes().first().row();
    }

    m_editor = new TMessagePartEditor(TMessagePart(), m_messagePartContainer->getItems(), this);
    connect(m_editor, &QWizard::finished, this, &TMessageEditor::onEditorFinished);
    m_editor->open();
}


void TMessageEditor::onRowDoubleClicked(const QModelIndex & index) {
    m_editedItemIndex = index.row();
    openEditor();
}

void TMessageEditor::onEditButtonClicked() {
    if(m_messagePartView->selectionModel()->selectedIndexes().isEmpty()) {
        return;
    }

    m_editedItemIndex = m_messagePartView->selectionModel()->selectedIndexes().first().row();
    openEditor();
}

void TMessageEditor::openEditor() {
    m_editor = new TMessagePartEditor(m_messagePartContainer->getItem(m_editedItemIndex), m_messagePartContainer->getItems(), this);
    connect(m_editor, &QWizard::finished, this, &TMessageEditor::onEditorFinished);
    m_editor->open();
}

void TMessageEditor::onMoveUpButtonClicked() {
    if(m_messagePartView->selectionModel()->selectedIndexes().isEmpty())
        return;

    int index = m_messagePartView->selectionModel()->selectedIndexes().first().row();
    
    if(m_messagePartContainer->swapItems(index, index-1)) {
        QModelIndex newIndex = m_messagePartView->model()->index(index-1, 0);
        m_messagePartView->selectionModel()->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }

    validateMessage();
}

void TMessageEditor::onMoveDownButtonClicked() {
    if(m_messagePartView->selectionModel()->selectedIndexes().isEmpty())
        return;

    int index = m_messagePartView->selectionModel()->selectedIndexes().first().row();
    
    if(m_messagePartContainer->swapItems(index, index+1)) {
        QModelIndex newIndex = m_messagePartView->model()->index(index+1, 0);
        m_messagePartView->selectionModel()->select(newIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }

    validateMessage();
}

void TMessageEditor::onEditorFinished(int finished) {
    if(finished != QDialog::Accepted) {
        return;
    }

    if(m_editedItemIndex >= 0) {
        m_messagePartContainer->updateItem(m_editedItemIndex, m_editor->messagePart());
    }
    else {
        m_messagePartContainer->addItem(m_addedItemIndex, m_editor->messagePart());
    }

    validateMessage();
}

void TMessageEditor::validateMessage() {
    const TMessage & validatedMessage = message();

    int messageCount = m_messagePartContainer->rowCount();
    for(int i = 0; i < messageCount; i++) {
        const QString & messageName = m_messagePartContainer->getItem(i).getName();
        m_messagePartContainer->updateItem(i, validatedMessage.getMessagePartByName(messageName));
    }
}


void TMessageEditor::onRemoveButtonClicked() {
    if(m_messagePartView->selectionModel()->selectedIndexes().isEmpty())
        return;

    m_messagePartContainer->removeItem(m_messagePartView->selectionModel()->selectedIndexes().first().row());
}


