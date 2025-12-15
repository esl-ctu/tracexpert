#ifndef TPROTOCOLEDITOR_H
#define TPROTOCOLEDITOR_H

#include <QWizard>
#include <QLineEdit>
#include <QFormLayout>
#include <QTableView>

#include "../protocol/tprotocol.h"
#include "../protocol/tprotocolmodel.h"
#include "../protocol/tprotocolcontainer.h"
#include "tmessagecontainer.h"
#include "tmessageeditor.h"

class TProtocolEditorDetailsPage : public QWizardPage {
    Q_OBJECT

public:
    explicit TProtocolEditorDetailsPage(const TProtocol * protocol, const TProtocolContainer * protocolContainer, QWidget * parent = nullptr);
    bool validatePage() override;

private:
    QLineEdit * m_nameLineEdit;

    const QString m_originalName;
    const TProtocolContainer * m_protocolContainer;
};

class TProtocolEditorWizard : public QWizard {
    Q_OBJECT

public:
    TProtocolEditorWizard(const TProtocolModel * protocolModel, TProtocolContainer * protocolContainer, QWidget * parent = nullptr);

protected:
    void accept() override;

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onRowDoubleClicked(const QModelIndex & index);
    void onRemoveButtonClicked();
    void onEditorFinished(int finished);

private:
    void openMessageEditor();

    TMessageEditor * m_messageEditor;
    qsizetype  m_editedMessageIndex;

    QTableView * m_messageView;
    TMessageContainer * m_messageContainer;

    const TProtocolModel * m_protocolModel;
    TProtocolContainer * m_protocolContainer;
};

#endif // TPROTOCOLEDITOR_H
