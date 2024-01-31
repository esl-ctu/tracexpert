#ifndef TMESSAGEFORMWIDGET_H
#define TMESSAGEFORMWIDGET_H


#include <QWidget>
#include "qformlayout.h"
#include "tmessage.h"

class TMessageFormWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TMessageFormWidget(QWidget * parent = nullptr);
    ~TMessageFormWidget();

    TMessage getMessage();
    void setMessage(const TMessage & message, bool * ok = nullptr);
    void resetLayout();
    bool assignInputValues();

public slots:
    void validateInputValues();

private:
    QWidget * createInputField(const TMessagePart & messagePart);

    TMessage m_message;
    QList<QWidget*> m_inputs;

    QFormLayout * m_formLayout;
};
#endif // TMESSAGEFORMWIDGET_H
