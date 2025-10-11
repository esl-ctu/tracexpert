#include "tmainwindow.h"
#include "tlogwidget.h"

#include <QApplication>
#include <QMutex>


static TLogWidget *gLogWidget = nullptr;
static QMutex gLogMutex;

void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    QMutexLocker locker(&gLogMutex);

    if (gLogWidget) {
        QMetaObject::invokeMethod(
            gLogWidget,
            "appendLogMessage",
            Qt::QueuedConnection,
            Q_ARG(QtMsgType, type),
            Q_ARG(QString, msg));
    }
}

int main(int argc, char * argv[])
{
    QApplication a(argc, argv);

    gLogWidget = new TLogWidget;
    qInstallMessageHandler(logMessageHandler);

    TMainWindow w(gLogWidget);
    w.show();
    return a.exec();
}
