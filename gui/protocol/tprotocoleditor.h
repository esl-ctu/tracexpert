#ifndef TPROTOCOLEDITOR_H
#define TPROTOCOLEDITOR_H

#include <QWizard>
#include <QLineEdit>
#include <QFormLayout>
#include <QTableView>

#include "protocol/tprotocolcontainer.h"
#include "tmessagesimplecontainer.h"
#include "tprotocol.h"
#include "tmessageeditor.h"

class TProtocolEditorDetailsPage : public QWizardPage {
    Q_OBJECT

public:
    explicit TProtocolEditorDetailsPage(const TProtocol & protocol, const TProtocolContainer * m_protocolContainer, QWidget * parent = nullptr);
    bool validatePage() override;

private:
    QLineEdit * m_nameLineEdit;

    const QString m_originalName;
    const TProtocolContainer * m_protocolContainer;
};

class TProtocolEditor : public QWizard {
    Q_OBJECT

public:
    TProtocolEditor(const TProtocol & protocol, const TProtocolContainer * m_protocolContainer, QWidget * parent = nullptr);

    TProtocol protocol();

signals:

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onRowDoubleClicked(const QModelIndex & index);
    void onRemoveButtonClicked();
    void onEditorFinished(int finished);

private:
    void openEditor();

    TMessageEditor * m_editor;
    qsizetype  m_editedItemIndex;

    QTableView * m_messageView;
    TMessageSimpleContainer * m_messageContainer;

    const TProtocol & m_protocol;
};

#endif // TPROTOCOLEDITOR_H
