#ifndef TLOGWIDGET_H
#define TLOGWIDGET_H

#include <QPlainTextEdit>
#include <QMetaObject>

#include <QDateTime>
#include <QTextCharFormat>
#include <QTextCursor>

class TLogWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    using QPlainTextEdit::QPlainTextEdit;

public slots:
    void appendLogMessage(QtMsgType type, const QString &msg);
};


#endif // TLOGWIDGET_H
