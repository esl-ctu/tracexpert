#ifndef TLOGHANDLER_H
#define TLOGHANDLER_H

#include <QPlainTextEdit>
#include <QMetaObject>

#include <QDateTime>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QMutex>

#include "tloglinewidget.h"

typedef QPlainTextEdit TLogWidget;

class TLogHandler : public QObject {
    Q_OBJECT
public:
    ~TLogHandler();

    static void installLogger();

    static TLogLineWidget * logLineWidget();
    static TLogWidget * logWidget();

    static void messageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg);

public slots:
    void appendLogMessage(QtMsgType type, const QString &msg);

private:
    TLogHandler();

    static inline TLogHandler * m_instance = nullptr;

    static inline QMutex m_logMutex;

    TLogLineWidget * m_logLineWidget;
    TLogWidget * m_logTextEdit;
};


#endif // TLOGHANDLER_H
