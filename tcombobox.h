#ifndef TCOMBOBOX_H
#define TCOMBOBOX_H

#include <QComboBox>

class TComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit TComboBox(QWidget * parent = nullptr);

    void wheelEvent(QWheelEvent * e) override;
};

#endif // TCOMBOBOX_H
