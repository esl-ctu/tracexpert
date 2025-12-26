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

#ifndef TTIMEEDIT_H
#define TTIMEEDIT_H

#include <QStringList>
#include <QLineEdit>
#include <QComboBox>

#define TTIMEEDITVER "cz.cvut.fit.TraceXpert.TTimeEdit/0.1"

class TTimeEdit : public QWidget
{
    Q_OBJECT

public:
    explicit TTimeEdit(QWidget * parent = nullptr);
    QString text() const;
    void setText(const QString & text);

signals:
    void textEdited(const QString & text);

private slots:
    void emitTextEdited();

private:
    static const QStringList TUnitNames;

    QLineEdit * m_lineEdit;
    QComboBox * m_comboBox;
};

#endif // TTIMEEDIT_H
