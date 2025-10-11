#include "tlogwidget.h"

void TLogWidget::appendLogMessage(QtMsgType type, const QString &msg) {

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
            fmt.setFontWeight(QFont::Bold); break;
        default:
            line = QString("[%1] %2").arg(time, msg);            ;
    }
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::End);
    c.insertText(line + '\n', fmt);
    this->moveCursor(QTextCursor::End);
    this->ensureCursorVisible();
}
