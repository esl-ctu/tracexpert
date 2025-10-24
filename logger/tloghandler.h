#ifndef TLOGHANDLER_H
#define TLOGHANDLER_H

#include <QPlainTextEdit>
#include <QMetaObject>

#include <QDateTime>
#include <QTextCharFormat>
#include <QTextCursor>

#include "tloglinewidget.h"

typedef QPlainTextEdit TLogWidget;

class TLogHandler : public QObject {
    Q_OBJECT
public:
    explicit TLogHandler();
    ~TLogHandler();

    TLogLineWidget * logLineWidget();
    TLogWidget * logWidget();

public slots:
    void appendLogMessage(QtMsgType type, const QString &msg);

protected:
    TLogLineWidget * m_logLineWidget;
    TLogWidget * m_logTextEdit;
};


#endif // TLOGHANDLER_H
