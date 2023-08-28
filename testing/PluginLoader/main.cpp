#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include <QtDebug>
#include <QFile>
#include <QTextStream>

#include "tplugin.h"

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
        break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
        break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
        //abort();
        break;
    default:
        txt = QString("%1").arg(msg);
    }
    QTextStream stream(stdout);
    QFile outFile("log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    stream << txt << Qt::endl;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInstallMessageHandler(myMessageHandler);
    qDebug("Start program");

    QTextStream stream(stdout);
    stream << "* Started plugin loader" << Qt::endl;

    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());

    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
        stream << "* FN: " << fileName;

        QObject *plugin = pluginLoader.instance();
        if (!plugin) {
            stream << " - ignoring" <<  Qt::endl;
        } else {
            stream << Qt::endl;
            TPlugin * aPlugin = qobject_cast<TPlugin *>(plugin);
            if (aPlugin){

                QString pluginId = fileName;
                if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);

                stream << "* Plug-in ID: '" << pluginId << "', name: '" << aPlugin->getPluginName() << "'\n";
                stream << "    Description: '" << aPlugin->getPluginInfo() << "'\n\n";
                stream.flush();

                if (pluginId.contains("serial")){
                    stream << "* Starting tests of the serial plugin" << Qt::endl;

                    TPlugin * serialPorts = aPlugin;

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

                if (pluginId.contains("newae")){
                    stream << "* Starting tests of the NewAE plugin" << Qt::endl;

                    TPlugin * newaePlug = aPlugin;
                    bool ok = false;
                    newaePlug->init(&ok);
                    if (!ok) {
                        stream << "** Load failed" << Qt::endl;
                    } else {
                        stream << "** Load ok" << Qt::endl;
                        QList<TScope *> a = newaePlug->getScopes();
                        stream << "*** Num devices: " << a.length() << Qt::endl;
                        if (a.length()){
                            a[0]->init(&ok);
                            if (!ok) {
                                stream << "** Scope ID 0 init failed" << Qt::endl;
                            } else {
                                stream << "** Scope ID 0 init ok" << Qt::endl;
                            }
                        }
                    }
                }

            }
        }
        pluginLoader.unload();
    }

    //return a.exec();
    return 0;
}
