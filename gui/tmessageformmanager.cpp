#include "tmessageformmanager.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <qcombobox>
#include <TDialog.h>
#include <QObjectCleanupHandler>


TMessageFormManager::TMessageFormManager(QFormLayout * formLayout, int insertOffset) {
    m_formLayout = formLayout;
    m_insertOffset = insertOffset;
}

TMessageFormManager::~TMessageFormManager() { }

void TMessageFormManager::clearRows() {

    for(QWidget * widget : m_inputs) {
        m_formLayout->removeRow(widget);
    }

    m_inputs.clear();
}

TMessage TMessageFormManager::getMessage() {
    return m_message;
}

void TMessageFormManager::setMessage(const TMessage & message, bool * ok) {
    clearRows();

    m_message = message;
    m_inputs.clear();

    int insertedInputs = 0;
    QList<TMessagePart> messageParts = m_message.getMessageParts();
    for(int i = 0; i < messageParts.size(); i++) {
        if(!messageParts[i].isPayload())
            continue;

        QWidget * input = createInputField(messageParts[i]);
        m_inputs.append(input);

        QLabel * label = new QLabel(messageParts[i].getName());
        label->setToolTip(messageParts[i].getDescription());

        m_formLayout->insertRow(m_insertOffset + insertedInputs, label, input);
        insertedInputs++;
    }

    if(insertedInputs == 0) {
        return;
    }

    // add a separator line
    QFrame * separatorLine = new QFrame();
    separatorLine->setFrameShape(QFrame::HLine);
    separatorLine->setFrameShadow(QFrame::Sunken);

    m_inputs.append(separatorLine);

    m_formLayout->insertRow(m_insertOffset, separatorLine);

    validateInputValues();
}

QWidget * TMessageFormManager::createInputField(const TMessagePart & messagePart) {
    QWidget * input;

    if (messagePart.getType() == TMessagePart::TType::TBool) {
        QComboBox * comboBox = new QComboBox;
        comboBox->addItem(tr("True"));
        comboBox->addItem(tr("False"));
        input = comboBox;
    }
    else if (messagePart.isHexOrAsciiSensibleType()) {
        QLineEdit * lineEdit = new QLineEdit;
        connect(lineEdit, &QLineEdit::textEdited, this, &TMessageFormManager::validateInputValues);

        QComboBox * comboBox = new QComboBox;
        comboBox->addItem(tr("Hex"));
        comboBox->addItem(tr("ASCII"));
        comboBox->setCurrentIndex((messagePart.getType() == TMessagePart::TType::TByte || messagePart.getType() == TMessagePart::TType::TByteArray) ? 0 : 1);
        connect(comboBox, &QComboBox::currentIndexChanged, this, &TMessageFormManager::validateInputValues);

        QHBoxLayout * layout = new QHBoxLayout;
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
        QLineEdit * lineEdit = new QLineEdit;
        connect(lineEdit, &QLineEdit::textEdited, this, &TMessageFormManager::validateInputValues);
        input = lineEdit;
    }

    return input;
}

bool TMessageFormManager::assignInputValues() {

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
            TDialog::parameterValueInvalid(m_formLayout->parentWidget(), messagePart.getName());
            return false;
        }

        payloadInputIndex++;
    }

    return true;
}

void TMessageFormManager::validateInputValues() {

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
