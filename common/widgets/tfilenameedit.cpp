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

#include "tfilenameedit.h"

#include <QApplication>
#include <QStyle>

TFileNameEdit::TFileNameEdit(QWidget * parent)
    : QLineEdit(parent)
{
    QAction * chooseFileNameAction = addAction(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton), QLineEdit::ActionPosition::TrailingPosition);
    connect(chooseFileNameAction, SIGNAL(triggered()), this, SLOT(chooseFileName()));
}

TFileNameEdit::TFileNameEdit(QFileDialog::FileMode mode, QWidget * parent)
    : TFileNameEdit(parent)
{
    m_mode = mode;
}

void TFileNameEdit::chooseFileName()
{
    QFileDialog fileDialog(this, "Choose file", text());
#ifdef FILEORDIR
    fileDialog.setOption(QFileDialog::DontUseNativeDialog, true);
    connect(&fileDialog, &QFileDialog::currentChanged,this,[&](const QString & str)
        {
            QFileInfo info(str);
            if(info.isFile())
                fileDialog.setFileMode(QFileDialog::ExistingFile);
            else if(info.isDir())
                fileDialog.setFileMode(QFileDialog::Directory);
        });
#else
    fileDialog.setFileMode(m_mode);
#endif
    if (fileDialog.exec()) {
        setText(fileDialog.selectedFiles().constFirst());
    };
}

