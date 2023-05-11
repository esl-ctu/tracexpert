#include <QApplication>

#include "tconfigparamwidgettest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    TConfigParamWidgetTest w;
    w.show();
    return a.exec();
}
