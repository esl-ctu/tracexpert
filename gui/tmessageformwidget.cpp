#include "tmessageformwidget.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <qcombobox>
#include <TDialog.h>
#include <QObjectCleanupHandler>


TMessageFormWidget::TMessageFormWidget(QWidget * parent) {
    m_formLayout = new QFormLayout();
    m_formLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_formLayout);
}

TMessageFormWidget::~TMessageFormWidget() { }

void TMessageFormWidget::resetLayout() {

    int rowCount = m_formLayout->rowCount();
    for(int i = 0; i < rowCount; i++) {
        m_formLayout->removeRow(rowCount-i-1);
    }

    QLabel * noFieldsLabel = new QLabel(tr("No message fields to fill in."));
    m_formLayout->addRow(noFieldsLabel);
    m_formLayout->setAlignment(noFieldsLabel, Qt::AlignCenter);
}

TMessage TMessageFormWidget::getMessage() {
    return m_message;
}

void TMessageFormWidget::setMessage(const TMessage & message, bool * ok) {
    resetLayout();

    m_message = message;
    m_inputs.clear();

    QList<TMessagePart> messageParts = m_message.getMessageParts();
    for(int i = 0; i < messageParts.size(); i++) {
        if(!messageParts[i].isPayload())
            continue;

        QWidget * input = createInputField(messageParts[i]);
        m_inputs.append(input);

        QLabel * label = new QLabel(messageParts[i].getName());
        label->setToolTip(messageParts[i].getDescription());

        m_formLayout->addRow(label, input);
    }

    if(m_formLayout->rowCount() > 1) {
        m_formLayout->removeRow(0);
        validateInputValues();
    }
}

QWidget * TMessageFormWidget::createInputField(const TMessagePart & messagePart) {
    QWidget * input;

    if (messagePart.getType() == TMessagePart::TType::TBool) {
        QComboBox * comboBox = new QComboBox(this);
        comboBox->addItem(tr("True"));
        comboBox->addItem(tr("False"));
        input = comboBox;
    }
    else if (messagePart.isHexOrAsciiSensibleType()) {
        QLineEdit * lineEdit = new QLineEdit();
        connect(lineEdit, &QLineEdit::textEdited, this, &TMessageFormWidget::validateInputValues);

        QComboBox * comboBox = new QComboBox();
        comboBox->addItem(tr("Hex"));
        comboBox->addItem(tr("ASCII"));
        comboBox->setCurrentIndex((messagePart.getType() == TMessagePart::TType::TByte || messagePart.getType() == TMessagePart::TType::TByteArray) ? 0 : 1);
        connect(comboBox, &QComboBox::currentIndexChanged, this, &TMessageFormWidget::validateInputValues);

        QHBoxLayout * layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(lineEdit);
        layout->addWidget(comboBox);
        layout->setStretch(0,4);
        layout->setStretch(1,1);

        QWidget * widget = new QWidget();
        widget->setLayout(layout);

        input = widget;
    }
    else {
        QLineEdit * lineEdit = new QLineEdit(this);
        connect(lineEdit, &QLineEdit::textEdited, this, &TMessageFormWidget::validateInputValues);
        input = lineEdit;
    }

    return input;
}

bool TMessageFormWidget::assignInputValues() {

    qsizetype payloadInputIndex = 0;
    QList<TMessagePart> & messageParts = m_message.getMessageParts();
    for(TMessagePart & messagePart : messageParts) {
        if(!messagePart.isPayload())
            continue;

        bool iok;
        QWidget * input = m_inputs[payloadInputIndex];

        if (messagePart.getType() == TMessagePart::TType::TBool) {
            QComboBox * comboBox = (QComboBox *)input;
            messagePart.setBool(comboBox->currentIndex() ? false : true, &iok);
        }
        else if (messagePart.isHexOrAsciiSensibleType()) {
            QWidget * widget = (QWidget *)input;

            QLineEdit * lineEdit = (QLineEdit *)widget->layout()->itemAt(0)->widget();
            QComboBox * comboBox = (QComboBox *)widget->layout()->itemAt(1)->widget();

            bool isAscii = comboBox->currentIndex();
            messagePart.setValue(lineEdit->text(), &iok, !isAscii, isAscii);
        }
        else {
            QLineEdit * lineEdit = (QLineEdit *)input;
            messagePart.setValue(lineEdit->text(), &iok);
        }

        if(!iok) {
            TDialog::parameterValueInvalid(this, messagePart.getName());
            return false;
        }

        payloadInputIndex++;
    }

    return true;
}

void TMessageFormWidget::validateInputValues() {

    qsizetype payloadInputIndex = 0;
    QList<TMessagePart> & messageParts = m_message.getMessageParts();
    for(TMessagePart & messagePart : messageParts) {
        if(!messagePart.isPayload())
            continue;

        bool iok;
        QWidget * input = m_inputs[payloadInputIndex];

        if (messagePart.getType() == TMessagePart::TType::TBool) {
            payloadInputIndex++;
            continue;
        }
        else if (messagePart.isHexOrAsciiSensibleType()) {
            QWidget * widget = (QWidget *)input;

            QLineEdit * lineEdit = (QLineEdit *)widget->layout()->itemAt(0)->widget();
            QComboBox * comboBox = (QComboBox *)widget->layout()->itemAt(1)->widget();

            bool isAscii = comboBox->currentIndex();
            messagePart.setValue(lineEdit->text(), &iok, !isAscii, isAscii);

            lineEdit->setStyleSheet(iok ? "background-color: white;" : "background-color: rgba(255, 0, 0, 0.3);");
        }
        else {
            QLineEdit * lineEdit = (QLineEdit *)input;
            messagePart.setValue(lineEdit->text(), &iok);

            lineEdit->setStyleSheet(iok ? "background-color: white;" : "background-color: rgba(255, 0, 0, 0.3);");
        }

        payloadInputIndex++;
    }
}
