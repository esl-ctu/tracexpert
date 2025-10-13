#include "tprotocolwidget.h"
#include "tprotocoltableview.h"

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
#include <QMessageBox>

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

    QFrame * line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    QPushButton * loadButton = new QPushButton("Import");
    connect(loadButton, &QPushButton::clicked, this, &TProtocolWidget::onLoadButtonClicked);

    QPushButton * saveButton = new QPushButton("Export");
    connect(saveButton, &QPushButton::clicked, this, &TProtocolWidget::onSaveButtonClicked);

    QVBoxLayout * sideButtonsLayout = new QVBoxLayout();
    sideButtonsLayout->addWidget(addMessageButton);
    sideButtonsLayout->addWidget(editMessageButton);
    sideButtonsLayout->addWidget(removeMessageButton);
    sideButtonsLayout->addWidget(line);
    sideButtonsLayout->addWidget(loadButton);
    sideButtonsLayout->addWidget(saveButton);
    sideButtonsLayout->addStretch();

    QTableView * protocolView = new TProtocolTableView();
    protocolView->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    m_protocolEditor = new TProtocolEditor(m_protocolContainer->at(m_editedItemIndex)->protocol(), m_protocolContainer, this);
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

void TProtocolWidget::onLoadButtonClicked() {
    try {
        int loadedCount = m_protocolContainer->loadProtocolFromFile();
        if(loadedCount > 0) {
            QMessageBox::information(this, tr("Protocol import success"), tr("Protocol(s) imported successfully."));
        }
    }
    catch(QString message) {
        QMessageBox::critical(this, tr("Protocol import failed"), tr("Unable to parse selected protocol file: %1").arg(message));
    }
}

void TProtocolWidget::onSaveButtonClicked() {
    if(m_protocolView->selectionModel()->selectedIndexes().isEmpty()) {
        QMessageBox::warning(this, tr("Protocol export failed"), tr("Select protocol(s) to be exported!"));
        return;
    }

    QModelIndexList rows = m_protocolView->selectionModel()->selectedRows();

    int totalSavedCount = 0;
    for (const QModelIndex &rowIndex : rows) {
        const TProtocolModel * protocolModel = m_protocolContainer->at(rowIndex.row());

        try {
            int savedCount = m_protocolContainer->saveProtocolToFile(protocolModel);
            totalSavedCount += savedCount;
        }
        catch(QString message) {
            QMessageBox::critical(this, tr("Protocol export failed"), tr("Unable to export selected protocol file: %1").arg(message));
            break;
        }
    }

    if(totalSavedCount > 0) {
        QMessageBox::information(this, tr("Protocol export success"), tr("Protocol(s) exported successfully."));
    }
}
