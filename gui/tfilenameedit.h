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
