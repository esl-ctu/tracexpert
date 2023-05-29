#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include "tplugin.h"

using namespace Qt;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTextStream stream(stdout);

    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());


    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));

        QObject *plugin = pluginLoader.instance();

		if (plugin) {

			TPlugin * filePlugin = qobject_cast<TPlugin *>(plugin);
			if (filePlugin) {

				QString pluginId = fileName;
				if(pluginId.startsWith("lib")) pluginId.remove(0, 3);
                if(pluginId.startsWith("sicak")) pluginId.remove(0, 5);
                if(pluginId.endsWith(".so")) pluginId.chop(3);
                if(pluginId.endsWith(".dll")) pluginId.chop(4);

                stream << "* Plug-in ID: '" << pluginId << "', name: '" << filePlugin->getPluginName() << "'\n";
                stream << "    Description: '" << filePlugin->getPluginInfo() << "'\n\n";
                stream.flush();

                stream << endl << "First generator: (bit-by-bit mode)" << endl;

                filePlugin->addIODevice("Random 1", "a bit-by-bit random generator");

                TIODevice * dummyGenerator = filePlugin->getIODevices().at(0);

				bool iok = false;
                dummyGenerator->init(&iok);

                stream << "Generator init: " << (iok ? "ok" : "not ok") << endl << endl;
				stream.flush();

                for(int i = 0; i < 5; i++)  {
                    uint8_t readBuffer[8] = {0, };
                    size_t readBytes = dummyGenerator->readData(readBuffer, 8);

                    uint64_t value = 0;
                    memcpy(&value, readBuffer, sizeof(uint64_t));

                    stream << "Read " << readBytes << " bytes: '" << hex << value << "'" << endl;
                }


                // second generator
                stream << endl << endl << "Second generator: (signed int)" << endl;

                filePlugin->addIODevice("Random 2", "an int-type random generator");

                dummyGenerator = filePlugin->getIODevices().at(1);

                TConfigParam preParams = dummyGenerator->getPreInitParams();

                QList<TConfigParam> preParamItems = preParams.getSubParams();
                stream << "Pre-init param names: " << "\n";
                for(int i = 0; i < preParamItems.size(); i++){
                    stream << preParamItems[i].getName() << "\n";
                }
                stream << endl << endl;

                preParams.getSubParamByName("Random number distribution")->setValue("uniform_int_distribution");
                preParams.getSubParamByName("Result data type")->setValue("int32");

                dummyGenerator->setPreInitParams(preParams);

                iok = false;
                dummyGenerator->init(&iok);

                stream << "Generator init: " << (iok ? "ok" : "not ok") << endl << endl;
                stream.flush();

                for(int i = 0; i < 5; i++)  {
                    uint8_t readBuffer[4] = {0, };
                    size_t readBytes = dummyGenerator->readData(readBuffer, 4);

                    int32_t value = 0;
                    memcpy(&value, readBuffer, sizeof(int32_t));

                    stream << "Read " << readBytes << " bytes: '" << dec << value << "'" << endl;
                }

                stream << endl << endl;

                TConfigParam postParams = dummyGenerator->getPostInitParams();

                QList<TConfigParam> postParamItems = postParams.getSubParams();
                stream << "Post-init param names: " << "\n";
                for(int i = 0; i < postParamItems.size(); i++){
                    stream << postParamItems[i].getName() << "\n";
                }
                stream << endl << endl;

                postParams.getSubParamByName("Minimum value (a)")->setValue("-5");
                postParams.getSubParamByName("Maximum value (b)")->setValue("5");

                dummyGenerator->setPostInitParams(postParams);

                for(int i = 0; i < 5; i++)  {
                    uint8_t readBuffer[4] = {0, };
                    size_t readBytes = dummyGenerator->readData(readBuffer, 4);

                    int32_t value = 0;
                    memcpy(&value, readBuffer, sizeof(int32_t));

                    stream << "Read " << readBytes << " bytes: '" << dec << value << "'" << endl;
                }

                stream << endl << endl;

			}
		}
		pluginLoader.unload();
	}

	return 0;
}
