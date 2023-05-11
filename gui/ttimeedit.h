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

    QLineEdit * lineEdit;
    QComboBox * comboBox;
};

#endif // TTIMEEDIT_H
