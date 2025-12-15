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
#include "../projectunit/tprojectunittableview.h"
#include "tmessageeditor.h"
#include "../tdialog.h"

TProtocolEditorDetailsPage::TProtocolEditorDetailsPage(const TProtocol * protocol, const TProtocolContainer * protocolContainer, QWidget * parent)
    : QWizardPage(parent), m_originalName(protocol->name()), m_protocolContainer(protocolContainer)  {

    setTitle("Protocol details");
    setSubTitle("Set protocol name and description");

    m_nameLineEdit = new QLineEdit(protocol->name());

    QLineEdit * descLineEdit = new QLineEdit(protocol->description());

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

TProtocolEditorWizard::TProtocolEditorWizard(const TProtocolModel * protocolModel, TProtocolContainer * protocolContainer, QWidget * parent)
    : QWizard(parent), m_protocolModel(protocolModel), m_protocolContainer(protocolContainer) {

    setWindowTitle("Protocol wizard");
    setOption(QWizard::CancelButtonOnLeft);

    addPage(new TProtocolEditorDetailsPage(protocolModel->protocol(), protocolContainer));

    QWizardPage * messageListPage = new QWizardPage;
    messageListPage->setTitle("Protocol messages");
    messageListPage->setSubTitle("Define protocol messages");

    QPushButton * addMessageButton = new QPushButton("Add");
    connect(addMessageButton, &QPushButton::clicked, this, &TProtocolEditorWizard::onAddButtonClicked);

    QPushButton * editMessageButton = new QPushButton("Edit");
    connect(editMessageButton, &QPushButton::clicked, this, &TProtocolEditorWizard::onEditButtonClicked);

    QPushButton * removeMessageButton = new QPushButton("Remove");
    connect(removeMessageButton, &QPushButton::clicked, this, &TProtocolEditorWizard::onRemoveButtonClicked);

    QVBoxLayout * sideButtonsLayout = new QVBoxLayout();
    sideButtonsLayout->addWidget(addMessageButton);
    sideButtonsLayout->addWidget(editMessageButton);
    sideButtonsLayout->addWidget(removeMessageButton);
    sideButtonsLayout->addStretch();

    QTableView * messageView = new TProjectUnitTableView();
    m_messageContainer = new TMessageContainer(protocolModel->protocol()->getMessages());
    messageView->setModel(m_messageContainer);
    messageView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
    connect(messageView, &QTableView::doubleClicked, this, &TProtocolEditorWizard::onEditButtonClicked);

    m_messageView = messageView;

    QHBoxLayout * horizontalLayout = new QHBoxLayout();
    horizontalLayout->addWidget(messageView);
    horizontalLayout->addLayout(sideButtonsLayout);

    messageListPage->setLayout(horizontalLayout);

    addPage(messageListPage);
}

void TProtocolEditorWizard::accept() {

    TProtocol * constructedProtocol = new TProtocol(
        field("name").toString(),
        field("description").toString()
    );

    int size = m_messageContainer->rowCount();
    for(int i = 0; i < size; i++) {
        constructedProtocol->addMessage(m_messageContainer->getItem(i));
    }

    int protocolIndex = m_protocolContainer->getIndexByName(m_protocolModel->name());
    if(protocolIndex >= 0) {
        if(!m_protocolContainer->update(protocolIndex, constructedProtocol)) {
            qWarning("Failed to update protocol, validation in editor probably failed.");
            delete constructedProtocol;
        }
    }
    else {
        if(!m_protocolContainer->add(constructedProtocol)) {
            qWarning("Failed to add protocol, validation in editor probably failed.");
            delete constructedProtocol;
        }
    }

    QWizard::accept();
}

void TProtocolEditorWizard::onAddButtonClicked() {
    m_editedMessageIndex = -1;

    m_messageEditor = new TMessageEditor(TMessage(), m_messageContainer->getItems(), this);
    connect(m_messageEditor, &QWizard::finished, this, &TProtocolEditorWizard::onEditorFinished);
    m_messageEditor->open();
}

void TProtocolEditorWizard::onRowDoubleClicked(const QModelIndex & index) {
    m_editedMessageIndex = index.row();
    openMessageEditor();
}

void TProtocolEditorWizard::onEditButtonClicked() {
    if(m_messageView->selectionModel()->selectedIndexes().isEmpty()) {
        return;
    }

    m_editedMessageIndex = m_messageView->selectionModel()->selectedIndexes().first().row();
    openMessageEditor();
}

void TProtocolEditorWizard::openMessageEditor() {
    m_messageEditor = new TMessageEditor(m_messageContainer->getItem(m_editedMessageIndex), m_messageContainer->getItems(), this);
    connect(m_messageEditor, &QWizard::finished, this, &TProtocolEditorWizard::onEditorFinished);
    m_messageEditor->open();
}

void TProtocolEditorWizard::onEditorFinished(int finished) {
    if(finished != QDialog::Accepted) {
        return;
    }

    if(m_editedMessageIndex >= 0) {
        m_messageContainer->updateItem(m_editedMessageIndex, m_messageEditor->message());
    }
    else {
        m_messageContainer->addItem(m_messageEditor->message());
    }

    m_messageContainer->sort();
}


void TProtocolEditorWizard::onRemoveButtonClicked() {
    if(m_messageView->selectionModel()->selectedIndexes().isEmpty())
        return;

    m_messageContainer->removeItem(m_messageView->selectionModel()->selectedIndexes().first().row());
}


