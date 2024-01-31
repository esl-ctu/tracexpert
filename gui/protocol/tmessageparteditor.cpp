#include "tmessageparteditor.h"

#include "qcombobox.h"
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
#include <TDialog.h>

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
    m_valueLineEdit->setEnabled(valueEditingAllowed);

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
    m_radioWidget->setEnabled(valueEditingAllowed && messagePart.isHexOrAsciiSensibleType());

    m_interpretedValueLineEdit = new QLineEdit();
    m_interpretedValueLineEdit->setEnabled(false);

    if(valueEditingAllowed) {
        m_interpretedValueLineEdit->setText(messagePart.getHumanReadableValue());
    }

    m_staticLengthCheckBox = new QCheckBox();
    m_staticLengthCheckBox->setChecked(messagePart.hasStaticLength());
    m_staticLengthCheckBox->setEnabled(variableLengthTypeSelected);

    m_lengthLineEdit = new QLineEdit();
    m_lengthLineEdit->setEnabled(messagePart.hasStaticLength() && variableLengthTypeSelected);
    if(!variableLengthTypeSelected) { m_lengthLineEdit->setText(QString::number(messagePart.getLength())); }

    m_dynamicLengthComboBox = new QComboBox();
    m_dynamicLengthComboBox->setEnabled(!messagePart.hasStaticLength());

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
        m_dynamicLengthComboBox->setCurrentIndex(messagePart.getLength());
    }

    QComboBox * endiannessComboBox = new QComboBox();
    endiannessComboBox->addItem(tr("Little endian"), true);
    endiannessComboBox->addItem(tr("Big endian"), false);
    endiannessComboBox->setCurrentIndex(messagePart.isLittleEndian() ? 0 : 1);

    QFormLayout * formLayout = new QFormLayout();
    formLayout->addRow(tr("&Name:"), nameLineEdit);
    formLayout->addRow(tr("&Description:"), descLineEdit);
    formLayout->addRow(tr("&Endianness:"), endiannessComboBox);
    formLayout->addRow(tr("&Is payload:"), m_payloadCheckBox);
    formLayout->addRow(tr("&Data type:"), typeComboBox);
    formLayout->addRow(tr("&Value:"), m_valueLineEdit);
    formLayout->addRow(tr("Interpret as:"), m_radioWidget);
    formLayout->addRow(tr("&Interpreted value:"), m_interpretedValueLineEdit);
    formLayout->addRow(tr("&Has static length:"), m_staticLengthCheckBox);
    formLayout->addRow(tr("&Length:"), m_lengthLineEdit);
    formLayout->addRow(tr("&Length determined by:"), m_dynamicLengthComboBox);

    registerField("name", nameLineEdit);
    registerField("description", descLineEdit);
    registerField("dataType", typeComboBox, "currentData", "currentIndexChanged");
    registerField("isPayload", m_payloadCheckBox);
    registerField("value", m_valueLineEdit);
    registerField("hasStaticLength", m_staticLengthCheckBox);
    registerField("length", m_lengthLineEdit);
    registerField("endianness", endiannessComboBox, "currentData", "currentIndexChanged");
    registerField("dynamicLength", m_dynamicLengthComboBox);
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

    setLayout(formLayout);
}

void TMessagePartEditorDetailsPage::updateDisplayedFields() {
    TMessagePart::TType type = TMessagePart::TType(field("dataType").value<int>());
    TMessagePart tmpMessagePart("", "", type);

    bool variableLengthTypeSelected = (type == TMessagePart::TType::TString || type == TMessagePart::TType::TByteArray);
    m_staticLengthCheckBox->setEnabled(variableLengthTypeSelected);

    if(!variableLengthTypeSelected) {
        m_lengthLineEdit->setText(QString::number(tmpMessagePart.getLength()));
        m_staticLengthCheckBox->setChecked(true);
    }

    bool staticLengthEditingAllowed = m_staticLengthCheckBox->isChecked() && variableLengthTypeSelected;
    m_lengthLineEdit->setEnabled(staticLengthEditingAllowed);
    m_dynamicLengthComboBox->setEnabled(!staticLengthEditingAllowed && variableLengthTypeSelected);

    if(staticLengthEditingAllowed) {
        m_dynamicLengthComboBox->setCurrentIndex(-1);
        tmpMessagePart.setLength(m_lengthLineEdit->text().toLongLong());
    }
    else if(variableLengthTypeSelected) {
        m_lengthLineEdit->setText("");
        tmpMessagePart.setStaticLength(false);
    }

    bool valueEditingAllowed = !m_payloadCheckBox->isChecked();
    m_valueLineEdit->setEnabled(valueEditingAllowed);
    m_radioWidget->setEnabled(valueEditingAllowed && tmpMessagePart.isHexOrAsciiSensibleType());

    if(valueEditingAllowed) {
        bool valueSetCorrectly;
        tmpMessagePart.setValue(m_valueLineEdit->text(),
                                &valueSetCorrectly,
                                tmpMessagePart.isHexOrAsciiSensibleType() && m_hexRadioButton->isChecked(),
                                tmpMessagePart.isHexOrAsciiSensibleType() && m_asciiRadioButton->isChecked());

        if(valueSetCorrectly) {
            bool isFormattedAsHex;
            QString interpretedValue = tmpMessagePart.getHumanReadableValue(isFormattedAsHex);
            m_interpretedValueLineEdit->setText(QString(isFormattedAsHex ? "0x" : "").append(interpretedValue));
        }
        else {
            m_interpretedValueLineEdit->setText("Error interpreting value. Check type and length.");
        }
    }
    else {
        m_valueLineEdit->setText("");
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

    if(m_lengthLineEdit->isEnabled()) {
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
