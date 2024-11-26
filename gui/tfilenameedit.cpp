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

