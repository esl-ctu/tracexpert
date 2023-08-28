#ifndef TFILENAMEEDIT_H
#define TFILENAMEEDIT_H

#include <QLineEdit>

#define TFILENAMEEDITVER "cz.cvut.fit.TraceXpert.TFileNameEdit/0.1"

class TFileNameEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit TFileNameEdit(QWidget * parent = nullptr);

private slots:
    void chooseFileName();
};

#endif // TFILENAMEEDIT_H
