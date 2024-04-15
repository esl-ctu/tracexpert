#ifndef TMESSAGEFORMWIDGET_H
#define TMESSAGEFORMWIDGET_H


#include <QWidget>
#include <QFormLayout>
#include <QFrame>
#include <QLineEdit>
#include "tmessage.h"

class TMessageFormManager : public QObject
{
    Q_OBJECT

public:
    explicit TMessageFormManager(QFormLayout * formLayout, int insertOffset);
    ~TMessageFormManager();

    TMessage getMessage();
    void setMessage(const TMessage & message, bool * ok = nullptr);
    void clearRows();
    bool assignInputValues();

public slots:
    void validateInputValues();

private:
    QWidget * createInputField(const TMessagePart & messagePart, bool isLengthDeterminingMessagePart);

    TMessage m_message;
    QMap<int, QWidget*> m_inputs;
    QList<int> m_lengthDeterminingMessagePartIndexes;
    QFrame * m_separatorLine = nullptr;

    QFormLayout * m_formLayout;
    int m_insertOffset;
};
#endif // TMESSAGEFORMWIDGET_H
