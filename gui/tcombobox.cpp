#include "tcombobox.h"

TComboBox::TComboBox(QWidget * parent) : QComboBox(parent)
{
    setFocusPolicy(Qt::StrongFocus);
}

void TComboBox::wheelEvent(QWheelEvent *e)
{
    if (hasFocus())
        QComboBox::wheelEvent(e);
}
