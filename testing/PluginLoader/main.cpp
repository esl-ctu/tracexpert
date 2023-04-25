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
            TPlugin * serialPorts = qobject_cast<TPlugin *>(plugin);
            if (serialPorts){

                QString pluginId = fileName;
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);

                stream << "* Plug-in ID: '" << pluginId << "', name: '" << serialPorts->getPluginName() << "'\n";
                stream << "    Description: '" << serialPorts->getPluginInfo() << "'\n\n";
                stream.flush();

                //TConfigParam test = serialPorts->getPreInitParams();
               // test.setValue("69, 420");
               // serialPorts->setPreInitParams(test);

                serialPorts->init();

                QList<TIODevice *> devices = serialPorts->getIODevices();

                for(int i = 0; i < devices.size(); i++){

                    stream << devices[i]->getIODeviceName() << "\n";
                    stream << devices[i]->getIODeviceInfo() << "\n";
                    stream.flush();

                }

                bool iok = false;

                devices[0]->init(&iok);
                //stream << iok << "\n";

                TConfigParam params = devices[0]->getPostInitParams();
                params.getSubParamByName("Baudrate")->setValue("Custom");
                params.getSubParamByName("Custom baudrate")->setValue("256000");
                devices[0]->setPostInitParams(params);

                params = devices[0]->getPostInitParams();
                stream << "Baudrate:" << params.getSubParamByName("Baudrate")->getValue() << "\n";
                stream.flush();

                uint8_t data[10] = {10, 1, 2, 3, 4, 5, 6, 7, 8, 9};

                devices[0]->writeData(data, 10);
                size_t read = devices[0]->readData(data, 10);
                if(read == 0) {
                    stream << "No data read" << "\n";
                } else {
                    data[read-1] = 0;
                    stream << (char *) data << "\n";
                }

            }
        }
        pluginLoader.unload();
    }

    //return a.exec();
    return 0;
}
