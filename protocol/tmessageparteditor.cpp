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

#include <QComboBox>
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

#include "qtimer.h"
#include "tmessageparteditor.h"
#include "../tdialog.h"

TMessagePartEditorDetailsPage::TMessagePartEditorDetailsPage(const TMessagePart & messagePart, const QList<TMessagePart> & messagePartList, QWidget * parent)
    : QWizardPage(parent), m_originalName(messagePart.getName()), m_messagePartList(messagePartList) {

    setTitle("Message part details");
    setSubTitle("Set message part name, type and length");

    QLineEdit * nameLineEdit = new QLineEdit(messagePart.getName());

    QLineEdit * descLineEdit = new QLineEdit(messagePart.getDescription());

    QComboBox * typeComboBox = new QComboBox();
    typeComboBox->addItem(tr("String"), (int)TMessagePart::TType::TString);
    typeComboBox->addItem(tr("Byte array"), (int)TMessagePart::TType::TByteArray);
    typeComboBox->addItem(tr("Boolean"), (int)TMessagePart::TType::TBool);
    typeComboBox->addItem(tr("Character"), (int)TMessagePart::TType::TChar);
    typeComboBox->addItem(tr("Unsigned char"), (int)TMessagePart::TType::TUChar);
    typeComboBox->addItem(tr("Byte"), (int)TMessagePart::TType::TByte);
    typeComboBox->addItem(tr("Short"), (int)TMessagePart::TType::TShort);
    typeComboBox->addItem(tr("Unsigned short"), (int)TMessagePart::TType::TUShort);
    typeComboBox->addItem(tr("Integer"), (int)TMessagePart::TType::TInt);
    typeComboBox->addItem(tr("Unsigned integer"), (int)TMessagePart::TType::TUInt);
    typeComboBox->addItem(tr("Long long"), (int)TMessagePart::TType::TLongLong);
    typeComboBox->addItem(tr("Unsigned long long"), (int)TMessagePart::TType::TULongLong);
    typeComboBox->addItem(tr("Real"), (int)TMessagePart::TType::TReal);
    typeComboBox->setCurrentIndex((int)messagePart.getType());

    m_payloadCheckBox = new QCheckBox();
    m_payloadCheckBox->setChecked(messagePart.isPayload());

    bool variableLengthTypeSelected = (messagePart.getType() == TMessagePart::TType::TString || messagePart.getType() == TMessagePart::TType::TByteArray);
    bool valueEditingAllowed = !m_payloadCheckBox->isChecked();

    m_valueLineEdit = new QLineEdit();

    bool isValueFormattedAsHex = false;
    if(valueEditingAllowed) {
        m_valueLineEdit->setText(messagePart.getHumanReadableValue(isValueFormattedAsHex));
    }

    bool isHexDefault = isValueFormattedAsHex || (messagePart.getType() == TMessagePart::TType::TByte && messagePart.getType() == TMessagePart::TType::TByteArray);

    m_hexRadioButton = new QRadioButton("Hex");
    m_hexRadioButton->setChecked(isHexDefault);

    m_asciiRadioButton = new QRadioButton("ASCII");
    m_asciiRadioButton->setChecked(!isHexDefault);

    m_radioWidget = new QWidget();
    QLayout * radioLayout = new QHBoxLayout();
    radioLayout->addWidget(m_hexRadioButton);
    radioLayout->addWidget(m_asciiRadioButton);
    radioLayout->setContentsMargins(0, 0, 0, 0);
    m_radioWidget->setLayout(radioLayout);

    m_interpretedValueLineEdit = new QLineEdit();
    m_interpretedValueLineEdit->setEnabled(false);

    if(valueEditingAllowed) {
        m_interpretedValueLineEdit->setText(messagePart.getHumanReadableValue());
    }

    m_staticLengthCheckBox = new QCheckBox();
    m_staticLengthCheckBox->setChecked(messagePart.hasStaticLength());

    m_lengthLineEdit = new QLineEdit();
    if(!variableLengthTypeSelected) { m_lengthLineEdit->setText(QString::number(messagePart.getLength())); }

    m_dynamicLengthComboBox = new QComboBox();

    qsizetype size = messagePartList.size();
    for(qsizetype i = 0; i < size; i++) {
        if(messagePartList[i].getName() == messagePart.getName()) {
            break;
        }

        if(messagePartList[i].hasLengthType()) {
            m_dynamicLengthComboBox->addItem(messagePartList[i].getName(), i);
        }
    }

    if(messagePart.hasStaticLength()) {
        m_lengthLineEdit->setText(QString::number(messagePart.getLength()));
        m_dynamicLengthComboBox->setCurrentIndex(-1);
    }
    else {
        if(messagePart.getLength() >= messagePartList.size()) {
            qDebug("Message part referenced by index %d does not exist.", messagePart.getLength());
        }
        else {
            m_dynamicLengthComboBox->setCurrentText(messagePartList[messagePart.getLength()].getName());
        }
    }

    m_endiannessComboBox = new QComboBox();
    m_endiannessComboBox->addItem(tr("Little endian"), true);
    m_endiannessComboBox->addItem(tr("Big endian"), false);
    m_endiannessComboBox->setCurrentIndex(messagePart.isLittleEndian() ? 0 : 1);

    m_formLayout = new QFormLayout(this);
    m_formLayout->addRow(tr("&Name:"), nameLineEdit);
    m_formLayout->addRow(tr("&Description:"), descLineEdit);
    m_formLayout->addRow(tr("&Data type:"), typeComboBox);
    m_formLayout->addRow(tr("&Is payload:"), m_payloadCheckBox);
    m_formLayout->addRow(tr("&Value:"), m_valueLineEdit);
    m_formLayout->addRow(tr("Interpret as:"), m_radioWidget);
    m_formLayout->addRow(tr("&Interpreted value:"), m_interpretedValueLineEdit);
    m_formLayout->addRow(tr("&Has static length:"), m_staticLengthCheckBox);
    m_formLayout->addRow(tr("&Length:"), m_lengthLineEdit);
    m_formLayout->addRow(tr("&Length determined by:"), m_dynamicLengthComboBox);
    m_formLayout->addRow(tr("&Endianness:"), m_endiannessComboBox);
    setLayout(m_formLayout);

    registerField("name", nameLineEdit);
    registerField("description", descLineEdit);
    registerField("dataType", typeComboBox, "currentData", "currentIndexChanged");
    registerField("isPayload", m_payloadCheckBox);
    registerField("value", m_valueLineEdit);
    registerField("hasStaticLength", m_staticLengthCheckBox);
    registerField("length", m_lengthLineEdit);
    registerField("endianness", m_endiannessComboBox, "currentData", "currentIndexChanged");
    registerField("dynamicLength", m_dynamicLengthComboBox, "currentData", "currentIndexChanged");
    registerField("asHex", m_hexRadioButton);
    registerField("asAscii", m_asciiRadioButton);

    connect(nameLineEdit, &QLineEdit::textEdited, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
    connect(typeComboBox, &QComboBox::currentIndexChanged, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
    connect(m_payloadCheckBox, &QCheckBox::stateChanged, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
    connect(m_valueLineEdit, &QLineEdit::textEdited, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
    connect(m_hexRadioButton, &QRadioButton::clicked, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
    connect(m_asciiRadioButton, &QRadioButton::clicked, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
    connect(m_staticLengthCheckBox, &QCheckBox::stateChanged, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
    connect(m_lengthLineEdit, &QLineEdit::textEdited, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);

    setFixedHeight(sizeHint().height());

    QTimer::singleShot(0, this, &TMessagePartEditorDetailsPage::updateDisplayedFields);
}

void TMessagePartEditorDetailsPage::updateDisplayedFields() {
    TMessagePart::TType type = TMessagePart::TType(field("dataType").value<int>());
    TMessagePart tmpMessagePart("", "", type);

    bool variableLengthTypeSelected = (type == TMessagePart::TType::TString || type == TMessagePart::TType::TByteArray);

    if(variableLengthTypeSelected ||
        type == TMessagePart::TType::TChar ||
        type == TMessagePart::TType::TUChar ||
        type == TMessagePart::TType::TByte ||
        type == TMessagePart::TType::TBool)
    {
        m_formLayout->setRowVisible(m_endiannessComboBox, false);
        m_endiannessComboBox->setCurrentIndex(0);
    }
    else {
        m_formLayout->setRowVisible(m_endiannessComboBox, true);
    }

    m_formLayout->setRowVisible(m_staticLengthCheckBox, variableLengthTypeSelected);

    bool isPayload = m_payloadCheckBox->isChecked();
    m_staticLengthCheckBox->setEnabled(variableLengthTypeSelected && isPayload);
    if(!variableLengthTypeSelected || !isPayload) {
        m_lengthLineEdit->setText(QString::number(tmpMessagePart.getLength()));
        m_staticLengthCheckBox->setChecked(true);
    }

    bool valueEditingAllowed = !isPayload;
    bool staticLengthEditingAllowed = m_staticLengthCheckBox->isChecked() && variableLengthTypeSelected;
    m_formLayout->setRowVisible(m_lengthLineEdit, staticLengthEditingAllowed);
    m_lengthLineEdit->setEnabled(staticLengthEditingAllowed && !valueEditingAllowed);
    m_formLayout->setRowVisible(m_dynamicLengthComboBox, !staticLengthEditingAllowed && variableLengthTypeSelected);

    if(staticLengthEditingAllowed) {
        m_dynamicLengthComboBox->setCurrentIndex(-1);

        if(!valueEditingAllowed) {
            tmpMessagePart.setLength(m_lengthLineEdit->text().toLongLong());
        }
        else {
            tmpMessagePart.setStaticLength(false);
        }
    }
    else if(variableLengthTypeSelected) {
        m_lengthLineEdit->setText("");
        tmpMessagePart.setStaticLength(false);
    }

    m_formLayout->setRowVisible(m_valueLineEdit, valueEditingAllowed);
    m_formLayout->setRowVisible(m_interpretedValueLineEdit, valueEditingAllowed);
    m_formLayout->setRowVisible(m_radioWidget, valueEditingAllowed && tmpMessagePart.isHexOrAsciiSensibleType());

    if(valueEditingAllowed) {
        bool valueSetCorrectly;
        tmpMessagePart.setValue(m_valueLineEdit->text(),
                                &valueSetCorrectly,
                                tmpMessagePart.isHexOrAsciiSensibleType() && m_hexRadioButton->isChecked(),
                                tmpMessagePart.isHexOrAsciiSensibleType() && m_asciiRadioButton->isChecked());

        if(valueSetCorrectly) {
            bool isFormattedAsHex;
            QString interpretedValue = tmpMessagePart.getHumanReadableValue(isFormattedAsHex, m_hexRadioButton->isChecked());
            m_interpretedValueLineEdit->setText(QString(isFormattedAsHex ? "0x" : "").append(interpretedValue));
            m_valueLineEdit->setStyleSheet("background-color: white;");
        }
        else {
            m_interpretedValueLineEdit->setText("Error interpreting value. Check type and length.");
            m_valueLineEdit->setStyleSheet("background-color: rgba(255, 0, 0, 0.3);");
        }

        if(staticLengthEditingAllowed && valueEditingAllowed) {
            m_lengthLineEdit->setText(QString::number(tmpMessagePart.getValue().size()));
        }
    }
    else {
        m_valueLineEdit->setText("");
        m_valueLineEdit->setStyleSheet("background-color: white;");
        m_interpretedValueLineEdit->setText("");
    }
}

bool TMessagePartEditorDetailsPage::validatePage() {
    if(field("name").toString().isEmpty()) {
        TDialog::parameterValueEmpty(this, tr("name"));
        return false;
    }

    if(!field("dataType").canConvert<int>()) {
        TDialog::parameterValueInvalid(this, tr("data type"));
        return false;
    }

    if(field("hasStaticLength").toBool()) {
        if(!field("length").canConvert<qsizetype>() || field("length").value<qsizetype>() < 1) {
            TDialog::parameterValueInvalid(this, tr("length"));
            return false;
        }
    }
    else {
        if(!field("dynamicLength").canConvert<qsizetype>()) {
            TDialog::parameterValueInvalid(this, tr("dynamic length"));
            return false;
        }
    }

    // if name was changed, check uniqueness
    bool isUnique = true;
    if(field("name").toString() != m_originalName) {
        for(const TMessagePart & messagePart : m_messagePartList) {
            if(field("name").toString() == messagePart.getName()) {
                isUnique = false;
                break;
            }
        }
    }

    if(!isUnique) {
        TDialog::parameterValueNotUniqueMessage(this, tr("name"));
        return false;
    }

    TMessagePart messagePart(
        field("name").toString(),
        field("description").toString(),
        TMessagePart::TType(field("dataType").value<int>()),
        field("isPayload").toBool(),
        QByteArray(),
        field("hasStaticLength").toBool(),
        field("hasStaticLength").toBool() ? field("length").value<qsizetype>() : field("dynamicLength").value<qsizetype>(),
        field("endianness").toBool()
        );

    if(!field("isPayload").toBool()) {
        bool valueSetCorrectly;
        messagePart.setValue(field("value").toString(), &valueSetCorrectly,
                             messagePart.isHexOrAsciiSensibleType() && field("asHex").toBool(),
                             messagePart.isHexOrAsciiSensibleType() && field("asAscii").toBool());
        if(!valueSetCorrectly) {
            TDialog::parameterValueInvalid(this, tr("value"));
            return false;
        }
    }

    return true;
}

TMessagePartEditor::TMessagePartEditor(const TMessagePart & messagePart, const QList<TMessagePart> & messagePartList, QWizard * parent) : QWizard(parent) {
    setWindowTitle("Message part wizard");
    setWizardStyle(QWizard::ModernStyle);
    TMessagePartEditorDetailsPage * detailsPage = new TMessagePartEditorDetailsPage(messagePart, messagePartList, this);
    addPage(detailsPage);
}

TMessagePart TMessagePartEditor::messagePart() {
    TMessagePart messagePart(
        field("name").toString(),
        field("description").toString(),
        TMessagePart::TType(field("dataType").value<int>()),
        field("isPayload").toBool(),
        QByteArray(),
        field("hasStaticLength").toBool(),
        field("hasStaticLength").toBool() ? field("length").value<qsizetype>() : field("dynamicLength").value<qsizetype>(),
        field("endianness").toBool()
    );

    if(!field("isPayload").toBool()) {
        messagePart.setValue(field("value").toString(), nullptr,
                             messagePart.isHexOrAsciiSensibleType() && field("asHex").toBool(),
                             messagePart.isHexOrAsciiSensibleType() && field("asAscii").toBool());
    }

    return messagePart;
}
