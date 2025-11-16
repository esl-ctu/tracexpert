#include "tmainwindow.h"
#include "logger/tloghandler.h"

#include <QApplication>
#include <QMutex>


static TLogHandler *gLogHandler = nullptr;
static QMutex gLogMutex;

void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QMutexLocker locker(&gLogMutex);

    if (gLogHandler) {
        QMetaObject::invokeMethod(
            gLogHandler,
            "appendLogMessage",
            Qt::QueuedConnection,
            Q_ARG(QtMsgType, type),
            Q_ARG(QString, msg));
    }
}

int main(int argc, char * argv[])
{

    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0"); // disable dark mode on Windows until the wrong color palette is resolved

    QApplication a(argc, argv);

    gLogHandler = new TLogHandler;
    qInstallMessageHandler(logMessageHandler);

    TMainWindow w(gLogHandler);
    w.show();
    return a.exec();
}
