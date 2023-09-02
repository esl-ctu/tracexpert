#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include "tplugin.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTextStream stream(stdout);

    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            TPlugin * emptyPlugin = qobject_cast<TPlugin *>(plugin);
            if (emptyPlugin){            

                stream << "* Plug-in filename: '" << fileName << "', name: '" << emptyPlugin->getPluginName() << "'\n";
                stream << "    Description: '" << emptyPlugin->getPluginInfo() << "'\n\n";
                stream.flush();             

            }
        }
        pluginLoader.unload();
    }

    //return a.exec();
    return 0;
}
