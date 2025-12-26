// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Adam Švehla (initial author)
// Vojtěch Miškovský
// Petr Socha

#include "tloghandler.h"

TLogHandler::TLogHandler()
{
    m_logLineWidget = new TLogLineWidget;
    m_logTextEdit = new TLogWidget;
}

TLogHandler::~TLogHandler()
{

}

void TLogHandler::installLogger()
{
    if (!m_instance)
        m_instance = new TLogHandler;

    qInstallMessageHandler(TLogHandler::messageHandler);
}

TLogLineWidget * TLogHandler::logLineWidget()
{
    if (!m_instance) {
        qWarning("Logger not installed!");
        return nullptr;
    }

    return m_instance->m_logLineWidget;
}

TLogWidget * TLogHandler::logWidget()
{
    if (!m_instance) {
        qWarning("Logger not installed!");
        return nullptr;
    }

    return m_instance->m_logTextEdit;
}

void TLogHandler::messageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QMutexLocker locker(&m_logMutex);

    if (!m_instance) {
        qWarning("Logger not installed!");
        return;
    }

    QMetaObject::invokeMethod(
        m_instance,
        "appendLogMessage",
        Qt::QueuedConnection,
        Q_ARG(QtMsgType, type),
        Q_ARG(QString, msg));
}

void TLogHandler::appendLogMessage(QtMsgType type, const QString &msg)
{

    #ifndef QT_DEBUG
    if (type == QtDebugMsg)
        return;
    #endif

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

    m_logLineWidget->setText(line);

    QTextCursor c = m_logTextEdit->textCursor();
    c.movePosition(QTextCursor::End);
    c.insertText(line + '\n', fmt);
    m_logTextEdit->moveCursor(QTextCursor::End);
    m_logTextEdit->ensureCursorVisible();
}
