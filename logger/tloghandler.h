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

    static TLogLineWidget * logLineWidget();
    static TLogWidget * logWidget();

    static void messageHandler(QtMsgType type, const QMessageLogContext & context, const QString & msg);

public slots:
    static void appendLogMessage(QtMsgType type, const QString &msg);

private:
    TLogHandler();

    static inline TLogHandler * m_instance = nullptr;

    static inline QMutex m_logMutex;

    static inline TLogLineWidget * m_logLineWidget = nullptr;
    static inline TLogWidget * m_logTextEdit = nullptr;
};


#endif // TLOGHANDLER_H
