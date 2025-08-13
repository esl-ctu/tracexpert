#ifndef TMESSAGEEDITOR_H
#define TMESSAGEEDITOR_H

#include <QWizard>
#include <QTableView>
#include <QLineEdit>
#include <QFormLayout>
#include <QComboBox>

#include "tmessage.h"
#include "tmessagepartsimplecontainer.h"
#include "tmessageparteditor.h"

class TMessageEditorDetailsPage : public QWizardPage {
    Q_OBJECT

public:
    TMessageEditorDetailsPage(const TMessage & message, const QList<TMessage> & messageList, QWidget * parent = nullptr);
    bool validatePage() override;

private:
    QLineEdit * m_nameLineEdit;

    QString m_originalName;
    const QList<TMessage> & m_messageList;
};



class TMessageEditor : public QWizard {
    Q_OBJECT

public:
    explicit TMessageEditor(const TMessage & message, const QList<TMessage> & messageList, QWidget * parent = nullptr);
    TMessage message();

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onRowDoubleClicked(const QModelIndex & index);
    void onRemoveButtonClicked();
    void onMoveUpButtonClicked();
    void onMoveDownButtonClicked();

    void onEditorFinished(int finished);

    void validateMessage();

private:
    void openEditor();

    TMessagePartEditor * m_editor;
    qsizetype  m_editedItemIndex;
    qsizetype  m_addedItemIndex;

    QTableView * m_messagePartView;
    TMessagePartSimpleContainer * m_messagePartContainer;

    const TMessage & m_message;
};

#endif // TMESSAGEEDITOR_H
