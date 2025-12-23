#include "buildinfo.h"
#include "tmainwindow.h"
#include "logger/tloghandler.h"

#include <QApplication>

int main(int argc, char * argv[])
{
#if defined(Q_OS_WIN)
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0"); // disable dark mode on Windows until the wrong color palette is resolved
#endif

    QApplication a(argc, argv);

    TLogHandler::installLogger();

    QCoreApplication::setOrganizationName("org.cvut.fit");
    QCoreApplication::setApplicationName("TraceXpert");
    QCoreApplication::setApplicationVersion(TRACEXPERT_VERSION);

    TMainWindow w;
    w.show();
    return a.exec();
}
