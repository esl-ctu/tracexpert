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

#include "ttimeedit.h"

#include <QHBoxLayout>
#include <QtMath>

const QStringList TTimeEdit::TUnitNames = {"s", "ms", "μs", "ns", "ps"};

TTimeEdit::TTimeEdit(QWidget * parent)
    : QWidget(parent)
{
    m_lineEdit = new QLineEdit(this);
    m_lineEdit->setValidator(new QDoubleValidator(this));
    connect(m_lineEdit, &QLineEdit::textEdited, this, &TTimeEdit::emitTextEdited);
    
    m_comboBox = new QComboBox(this);
    m_comboBox->addItems(TUnitNames);
    connect(m_comboBox, &QComboBox::currentIndexChanged, this, &TTimeEdit::emitTextEdited);

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_comboBox);

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout);
}

QString TTimeEdit::text() const
{
    qreal value = m_lineEdit->validator()->locale().toDouble(m_lineEdit->text());
    value *= qPow(10, -3 * m_comboBox->currentIndex());
    return QString::number(value);
}

void TTimeEdit::setText(const QString & text)
{
    qreal value = text.toDouble();
    int unit = 0;
    while (unit < TUnitNames.size()-1 && value < 1 && value > -1 && value) {
        value *= 1000;
        unit++;
    }
    m_lineEdit->setText(m_lineEdit->validator()->locale().toString(value));
    m_comboBox->setCurrentIndex(unit);
}

void TTimeEdit::emitTextEdited()
{
    emit textEdited(text());
}
