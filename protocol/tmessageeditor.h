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

#ifndef TMESSAGEEDITOR_H
#define TMESSAGEEDITOR_H

#include <QWizard>
#include <QTableView>
#include <QLineEdit>
#include <QFormLayout>
#include <QComboBox>

#include "tmessage.h"
#include "tmessagepartcontainer.h"
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
    TMessagePartContainer * m_messagePartContainer;

    const TMessage & m_message;
};

#endif // TMESSAGEEDITOR_H
