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
// Vojtěch Miškovský (initial author)

#include "tmessageformmanager.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QObjectCleanupHandler>

#include "../../tdialog.h"
#include "../../tpalette.h"


TMessageFormManager::TMessageFormManager(QFormLayout * formLayout, int insertOffset) {
    m_formLayout = formLayout;
    m_insertOffset = insertOffset;
}

TMessageFormManager::~TMessageFormManager() { }

void TMessageFormManager::clearRows() {
    if(m_separatorLine) {
        m_formLayout->removeRow(m_separatorLine);
        m_separatorLine = nullptr;
    }

    for(auto [index, widget] : m_inputs.asKeyValueRange()) {
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
    QList<TMessagePart> messageParts = m_message.getMessageParts();

    m_lengthDeterminingMessagePartIndexes.clear();
    for(int i = 0; i < messageParts.size(); i++) {
        if(messageParts[i].hasStaticLength()) {
            continue;
        }

        m_lengthDeterminingMessagePartIndexes.append(messageParts[i].getLength());
    }

    int insertedInputs = 0;
    for(int i = 0; i < messageParts.size(); i++) {
        if(!messageParts[i].isPayload()) {
            continue;
        }

        QWidget * input = createInputField(messageParts[i], m_lengthDeterminingMessagePartIndexes.contains(i));
        m_inputs.insert(i, input);

        QLabel * label = new QLabel(messageParts[i].getName());
        label->setToolTip(messageParts[i].getDescription());

        m_formLayout->insertRow(m_insertOffset + insertedInputs, label, input);
        insertedInputs++;
    }

    if(insertedInputs == 0) {
        return;
    }

    // add a separator line
    m_separatorLine = new QFrame();
    m_separatorLine->setFrameShape(QFrame::HLine);
    m_separatorLine->setFrameShadow(QFrame::Sunken);
    m_formLayout->insertRow(m_insertOffset, m_separatorLine);

    validateInputValues();
}

QWidget * TMessageFormManager::createInputField(const TMessagePart & messagePart, bool isLengthDeterminingMessagePart) {
    QWidget * input;

    if(messagePart.getType() == TMessagePart::TType::TBool) {
        QComboBox * comboBox = new QComboBox;
        comboBox->addItem(tr("True"));
        comboBox->addItem(tr("False"));
        input = comboBox;
    }
    else if(messagePart.isHexOrAsciiSensibleType() && !isLengthDeterminingMessagePart) {
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

    if(isLengthDeterminingMessagePart) {
        input->setEnabled(false);
    }

    return input;
}

bool TMessageFormManager::assignInputValues() {
    QList<TMessagePart> & messageParts = m_message.getMessageParts();
    for(int i = 0; i < messageParts.size(); i++) {
        if(!messageParts[i].isPayload()) {
            continue;
        }

        bool iok;
        QWidget * input = m_inputs[i];

        if(!input->isEnabled()) {
            continue;
        }

        if(messageParts[i].getType() == TMessagePart::TType::TBool) {
            QComboBox * comboBox = (QComboBox *)input;
            messageParts[i].setBool(comboBox->currentIndex() ? false : true, &iok);
        }
        else if(messageParts[i].isHexOrAsciiSensibleType()) {
            QWidget * widget = (QWidget *)input;

            QLineEdit * lineEdit = (QLineEdit *)widget->layout()->itemAt(0)->widget();
            QComboBox * comboBox = (QComboBox *)widget->layout()->itemAt(1)->widget();

            bool isAscii = comboBox->currentIndex();
            messageParts[i].setValue(lineEdit->text(), &iok, !isAscii, isAscii);
        }
        else {
            QLineEdit * lineEdit = (QLineEdit *)input;
            messageParts[i].setValue(lineEdit->text(), &iok);
        }

        if(iok && !messageParts[i].hasStaticLength()) {
            int lengthMessagePartIndex = messageParts[i].getLength();

            // value of length-determining field can change (is not static)
            if(m_inputs.contains(lengthMessagePartIndex)) {
                QLineEdit * lineEdit = (QLineEdit *)m_inputs[lengthMessagePartIndex];
                lineEdit->setText(QString::number(messageParts[i].getDataLength()));
                messageParts[lengthMessagePartIndex].setValue(lineEdit->text(), &iok, false, false, true);
            }
            // value of length-determining field is static
            else if(!messageParts[lengthMessagePartIndex].isPayload()) {
                iok = (messageParts[i].getDataLength() == messageParts[lengthMessagePartIndex].getValueAsLength());
            }
            // the value of length-determining field can change (isPayload) but an input field was not generated
            else {
                qWarning("Referenced length-determining message part could not be found.");
            }
        }

        if(!iok) {
            TDialog::parameterValueInvalid(m_formLayout->parentWidget(), messageParts[i].getName());
            return false;
        }
    }

    return true;
}

void TMessageFormManager::validateInputValues()
{
    std::function<void(QLineEdit *,bool)> setBackground = [&](QLineEdit * lineEdit, bool isOk) {
        QColor backgroundColor = isOk ? QGuiApplication::palette().color(QPalette::Base) : TPalette::color(TPalette::ErrorBase);
        lineEdit->setStyleSheet(QString("background-color: %1;").arg(backgroundColor.name()));
    };

    QList<TMessagePart> & messageParts = m_message.getMessageParts();
    for(int i = 0; i < messageParts.size(); i++) {
        if(!messageParts[i].isPayload()) {
            continue;
        }

        bool iok;
        QWidget * input = m_inputs[i];

        if(!input->isEnabled()) {
            continue;
        }

        QLineEdit * lineEdit;
        if(messageParts[i].getType() == TMessagePart::TType::TBool) {
            continue;
        }
        else if(messageParts[i].isHexOrAsciiSensibleType()) {
            QWidget * widget = (QWidget *)input;

            lineEdit = (QLineEdit *)widget->layout()->itemAt(0)->widget();
            QComboBox * comboBox = (QComboBox *)widget->layout()->itemAt(1)->widget();

            bool isAscii = comboBox->currentIndex();
            messageParts[i].setValue(lineEdit->text(), &iok, !isAscii, isAscii);

            setBackground(lineEdit, iok);
        }
        else {
            lineEdit = (QLineEdit *)input;
            messageParts[i].setValue(lineEdit->text(), &iok);

            setBackground(lineEdit, iok);
        }

        // if this message part's length is determined by another message part
        if(!messageParts[i].hasStaticLength()) {
            int lengthMessagePartIndex = messageParts[i].getLength();

            // value of length-determining field can change (is not static)
            if(m_inputs.contains(lengthMessagePartIndex)) {
                QLineEdit * lineEdit = (QLineEdit *)m_inputs[lengthMessagePartIndex];
                lineEdit->setText(QString::number(messageParts[i].getDataLength()));
                messageParts[lengthMessagePartIndex].setValue(lineEdit->text(), &iok);
            }
            // value of length-determining field is static
            else if(!messageParts[lengthMessagePartIndex].isPayload()) {
                iok = (messageParts[i].getDataLength() == messageParts[lengthMessagePartIndex].getValueAsLength());
                setBackground(lineEdit, iok);
            }
            // the value of length-determining field can change (isPayload) but an input field was not generated
            else {
                qWarning("Referenced length-determining message part could not be found.");
            }
        }
    }
}
