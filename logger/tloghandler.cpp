#include "tloghandler.h"

TLogHandler::TLogHandler()
{

}

TLogHandler::~TLogHandler()
{

}

TLogLineWidget * TLogHandler::logLineWidget()
{
    if (!m_logLineWidget)
        m_logLineWidget = new TLogLineWidget;

    return m_logLineWidget;
}

TLogWidget * TLogHandler::logWidget()
{
    if (!m_logTextEdit)
        m_logTextEdit = new TLogWidget;

    return m_logTextEdit;
}

void TLogHandler::messageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QMutexLocker locker(&m_logMutex);

    if (!m_instance)
        m_instance = new TLogHandler;

    QMetaObject::invokeMethod(
        m_instance,
        "appendLogMessage",
        Qt::QueuedConnection,
        Q_ARG(QtMsgType, type),
        Q_ARG(QString, msg));
}

void TLogHandler::appendLogMessage(QtMsgType type, const QString &msg)
{
    if(msg.startsWith("QWindowsWindow")) return; // ugly workaround, TODO: message filtering system

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString line;

    QTextCharFormat fmt;
    switch (type) {
        case QtDebugMsg:
            line = QString("[%1] DEBUG %2").arg(time, msg);
            break;
        case QtInfoMsg:
            line = QString("[%1] INFO %2").arg(time, msg);
            break;
        case QtWarningMsg:
            line = QString("[%1] WARNING %2").arg(time, msg);
            break;
        case QtCriticalMsg:
            line = QString("[%1] ERROR %2").arg(time, msg);
            fmt.setFontWeight(QFont::Bold);
            break;
        default:
            line = QString("[%1] %2").arg(time, msg);
            break;
    }

    logLineWidget()->setText(line);

    QTextCursor c = logWidget()->textCursor();
    c.movePosition(QTextCursor::End);
    c.insertText(line + '\n', fmt);
    logWidget()->moveCursor(QTextCursor::End);
    logWidget()->ensureCursorVisible();
}
