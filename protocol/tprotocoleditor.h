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
