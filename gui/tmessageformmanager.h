#ifndef TMESSAGEFORMWIDGET_H
#define TMESSAGEFORMWIDGET_H


#include <QWidget>
#include "qformlayout.h"
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
    QWidget * createInputField(const TMessagePart & messagePart);

    TMessage m_message;
    QList<QWidget*> m_inputs;

    QFormLayout * m_formLayout;
    int m_insertOffset;
};
#endif // TMESSAGEFORMWIDGET_H
