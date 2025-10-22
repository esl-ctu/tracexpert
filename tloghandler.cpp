#include "tloghandler.h"

TLogHandler::TLogHandler()
{
    m_logLineWidget = new TLogLineWidget();
    m_logTextEdit = new QPlainTextEdit();
}

TLogHandler::~TLogHandler()
{
    delete m_logLineWidget;
    delete m_logTextEdit;
}

TLogLineWidget * TLogHandler::logLineWidget() {
    return m_logLineWidget;
}

TLogWidget * TLogHandler::logWidget() {
    return m_logTextEdit;
}

void TLogHandler::appendLogMessage(QtMsgType type, const QString &msg) {

    if(msg.startsWith("QWindowsWindow")) return; // ugly workaround, TODO: message filtering system

    QString time = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString line;

    QTextCharFormat fmt;
    switch (type) {
        case QtDebugMsg:
            line = QString("[%1] DEBUG %2").arg(time, msg);
            m_logLineWidget->setStyleSheet("QLabel { color : grey; }");
            break;
        case QtInfoMsg:
            line = QString("[%1] INFO %2").arg(time, msg);
            m_logLineWidget->setStyleSheet("QLabel { color : blue; }");
            break;
        case QtWarningMsg:
            line = QString("[%1] WARNING %2").arg(time, msg);
            m_logLineWidget->setStyleSheet("QLabel { color : orange; }");
            break;
        case QtCriticalMsg:
            line = QString("[%1] ERROR %2").arg(time, msg);
            fmt.setFontWeight(QFont::Bold); break;
            m_logLineWidget->setStyleSheet("QLabel { color : red; }");
        default:
            line = QString("[%1] %2").arg(time, msg);
            m_logLineWidget->setStyleSheet("");
    }

    m_logLineWidget->setText(line);

    QTextCursor c = m_logTextEdit->textCursor();
    c.movePosition(QTextCursor::End);
    c.insertText(line + '\n', fmt);
    m_logTextEdit->moveCursor(QTextCursor::End);
    m_logTextEdit->ensureCursorVisible();
}
