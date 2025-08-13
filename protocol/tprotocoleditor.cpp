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

#include "tprotocoleditor.h"
#include "tprotocoltableview.h"
#include "tmessageeditor.h"
#include "../tdialog.h"

TProtocolEditorDetailsPage::TProtocolEditorDetailsPage(const TProtocol & protocol, const TProtocolContainer * protocolContainer, QWidget * parent)
    : QWizardPage(parent), m_originalName(protocol.getName()), m_protocolContainer(protocolContainer)  {

    setTitle("Protocol details");
    setSubTitle("Set protocol name and description");

    m_nameLineEdit = new QLineEdit(protocol.getName());

    QLineEdit * descLineEdit = new QLineEdit(protocol.getDescription());

    QFormLayout * formLayout = new QFormLayout();
    formLayout->addRow(tr("&Name:"), m_nameLineEdit);
    formLayout->addRow(tr("&Description:"), descLineEdit);

    registerField("name", m_nameLineEdit);
    registerField("description", descLineEdit);

    setLayout(formLayout);
}

bool TProtocolEditorDetailsPage::validatePage() {
    if(field("name").toString().isEmpty()) {
        TDialog::parameterValueEmpty(this, tr("name"));
        return false;
    }

    // if name was changed, check uniqueness
    bool nameFound = false;
    if(field("name").toString() != m_originalName) {
        m_protocolContainer->getByName(field("name").toString(), &nameFound);
    }

    if(nameFound) {
        TDialog::parameterValueNotUniqueMessage(this, tr("name"));
        return false;
    }

    return true;
}

TProtocolEditor::TProtocolEditor(const TProtocol & protocol, const TProtocolContainer * protocolContainer, QWidget * parent) : QWizard(parent), m_protocol(protocol) {

    setWindowTitle("Protocol wizard");
    setOption(QWizard::CancelButtonOnLeft);

    addPage(new TProtocolEditorDetailsPage(protocol, protocolContainer));

    QWizardPage * messageListPage = new QWizardPage;
    messageListPage->setTitle("Protocol messages");
    messageListPage->setSubTitle("Define protocol messages");

    QPushButton * addMessageButton = new QPushButton("Add");
    connect(addMessageButton, &QPushButton::clicked, this, &TProtocolEditor::onAddButtonClicked);

    QPushButton * editMessageButton = new QPushButton("Edit");
    connect(editMessageButton, &QPushButton::clicked, this, &TProtocolEditor::onEditButtonClicked);

    QPushButton * removeMessageButton = new QPushButton("Remove");
    connect(removeMessageButton, &QPushButton::clicked, this, &TProtocolEditor::onRemoveButtonClicked);

    QVBoxLayout * sideButtonsLayout = new QVBoxLayout();
    sideButtonsLayout->addWidget(addMessageButton);
    sideButtonsLayout->addWidget(editMessageButton);
    sideButtonsLayout->addWidget(removeMessageButton);
    sideButtonsLayout->addStretch();

    QTableView * messageView = new TProtocolTableView();
    m_messageContainer = new TMessageSimpleContainer(protocol.getMessages());
    messageView->setModel(m_messageContainer);
    messageView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
    connect(messageView, &QTableView::doubleClicked, this, &TProtocolEditor::onEditButtonClicked);

    m_messageView = messageView;

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(messageView);
    horizontalLayout->addLayout(sideButtonsLayout);

    messageListPage->setLayout(horizontalLayout);

    addPage(messageListPage);
}

TProtocol TProtocolEditor::protocol() {
    TProtocol constructedProtocol(
        field("name").toString(),
        field("description").toString()
    );

    int size = m_messageContainer->rowCount();
    for(int i = 0; i < size; i++) {
        constructedProtocol.addMessage(m_messageContainer->getItem(i));
    }

    return constructedProtocol;
}

void TProtocolEditor::onAddButtonClicked() {
    m_editedItemIndex = -1;

    m_editor = new TMessageEditor(TMessage(), m_messageContainer->getItems(), this);
    connect(m_editor, &QWizard::finished, this, &TProtocolEditor::onEditorFinished);
    m_editor->open();
}

void TProtocolEditor::onRowDoubleClicked(const QModelIndex & index) {
    m_editedItemIndex = index.row();
    openEditor();
}

void TProtocolEditor::onEditButtonClicked() {
    if(m_messageView->selectionModel()->selectedIndexes().isEmpty()) {
        return;
    }

    m_editedItemIndex = m_messageView->selectionModel()->selectedIndexes().first().row();
    openEditor();
}

void TProtocolEditor::openEditor() {
    m_editor = new TMessageEditor(m_messageContainer->getItem(m_editedItemIndex), m_messageContainer->getItems(), this);
    connect(m_editor, &QWizard::finished, this, &TProtocolEditor::onEditorFinished);
    m_editor->open();
}

void TProtocolEditor::onEditorFinished(int finished) {
    if(finished != QDialog::Accepted) {
        return;
    }

    if(m_editedItemIndex >= 0) {
        m_messageContainer->updateItem(m_editedItemIndex, m_editor->message());
    }
    else {
        m_messageContainer->addItem(m_editor->message());
    }

    m_messageContainer->sort();
}


void TProtocolEditor::onRemoveButtonClicked() {
    if(m_messageView->selectionModel()->selectedIndexes().isEmpty())
        return;

    m_messageContainer->removeItem(m_messageView->selectionModel()->selectedIndexes().first().row());
}


