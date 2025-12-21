#include "buildinfo.h"
#include "tmainwindow.h"
#include "logger/tloghandler.h"

#include <QApplication>

int main(int argc, char * argv[])
{
    qputenv("QT_QPA_PLATFORM", "windows:darkmode=0"); // disable dark mode on Windows until the wrong color palette is resolved

    QApplication a(argc, argv);

    TLogHandler::installLogger();

    QCoreApplication::setOrganizationName("org.cvut.fit");
    QCoreApplication::setApplicationName("TraceXpert");
    QCoreApplication::setApplicationVersion(TRACEXPERT_VERSION);

    TMainWindow w;
    w.show();
    return a.exec();
}
