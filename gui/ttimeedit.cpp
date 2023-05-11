#include "ttimeedit.h"

#include <QHBoxLayout>
#include <QtMath>

const QStringList TTimeEdit::TUnitNames = {"s", "ms", "μs", "ns", "ps"};

TTimeEdit::TTimeEdit(QWidget * parent)
    : QWidget{parent}
{
    lineEdit = new QLineEdit(this);
    lineEdit->setValidator(new QDoubleValidator(this));
    connect(lineEdit, &QLineEdit::textEdited, this, &TTimeEdit::emitTextEdited);

    comboBox = new QComboBox(this);
    comboBox->addItems(TUnitNames);
    connect(comboBox, &QComboBox::currentIndexChanged, this, &TTimeEdit::emitTextEdited);

    QHBoxLayout * layout = new QHBoxLayout(this);
    layout->addWidget(lineEdit);
    layout->addWidget(comboBox);

    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout);
}

QString TTimeEdit::text() const
{
    qreal value = lineEdit->text().toDouble();
    value *= qPow(10, -3 * comboBox->currentIndex());
    return QString::number(value);
}

void TTimeEdit::setText(const QString &text)
{
    qreal value = text.toDouble();
    int unit = 0;
    while (unit < TUnitNames.size()-1 && value < 1 && value > -1 && value) {
        value *= 1000;
        unit++;
    }
    lineEdit->setText(QString::number(value));
    comboBox->setCurrentIndex(unit);
}

void TTimeEdit::emitTextEdited()
{
    emit textEdited(text());
}
