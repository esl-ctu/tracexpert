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
// Vojtěch Miškovský (initial author)

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
