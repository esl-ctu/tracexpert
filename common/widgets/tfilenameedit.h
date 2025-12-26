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

#ifndef TFILENAMEEDIT_H
#define TFILENAMEEDIT_H

#include <QLineEdit>
#include <QFileDialog>

#define TFILENAMEEDITVER "cz.cvut.fit.TraceXpert.TFileNameEdit/0.1"

class TFileNameEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit TFileNameEdit(QWidget * parent = nullptr);
    explicit TFileNameEdit(QFileDialog::FileMode mode, QWidget * parent = nullptr);

private slots:
    void chooseFileName();

private:
    QFileDialog::FileMode m_mode = QFileDialog::AnyFile;
};

#endif // TFILENAMEEDIT_H
