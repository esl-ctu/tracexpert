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

#ifndef TMESSAGEPARTEDITOR_H
#define TMESSAGEPARTEDITOR_H

#include <QWizard>
#include <QTableView>
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
#include <QCheckBox>
#include <QComboBox>

#include "tmessagepart.h"

class TMessagePartEditorDetailsPage : public QWizardPage {
    Q_OBJECT

public:
    TMessagePartEditorDetailsPage(const TMessagePart & messagePart, const QList<TMessagePart> & messagePartList, QWidget * parent = nullptr);
    bool validatePage() override;
    void updateDisplayedFields();

private:
    QCheckBox * m_payloadCheckBox;
    QWidget   * m_radioWidget;
    QLineEdit * m_valueLineEdit;
    QRadioButton * m_hexRadioButton;
    QRadioButton * m_asciiRadioButton;
    QLineEdit * m_interpretedValueLineEdit;
    QCheckBox * m_staticLengthCheckBox;
    QLineEdit * m_lengthLineEdit;
    QComboBox * m_dynamicLengthComboBox;
    QComboBox * m_endiannessComboBox;

    QFormLayout * m_formLayout;

    const QString m_originalName;
    const QList<TMessagePart> & m_messagePartList;
};

class TMessagePartEditor : public QWizard {
    Q_OBJECT

public:
    explicit TMessagePartEditor(const TMessagePart & messagePart, const QList<TMessagePart> & messagePartList, QWizard * parent = nullptr);
    TMessagePart messagePart();
};

#endif // TMESSAGEPARTEDITOR_H
