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


                filePlugin->addIODevice("dummy.txt", "a test file");

                TIODevice * dummyFile = filePlugin->getIODevices().at(0);

                TConfigParam preParams = dummyFile->getPreInitParams();
                stream << "File path: " << preParams.getSubParamByName("File path")->getValue() << "\n\n";

                QList<TConfigParam> subParams = preParams.getSubParams();
                stream << "Pre-init param names: " << "\n";
                for(int i = 0; i < subParams.size(); i++){
                    stream << subParams[i].getName() << "\n";
				}
				stream << "\n\n";

				preParams.getSubParamByName("Read/Write mode")->setValue("ReadWrite");
				preParams.getSubParamByName("Write behaviour")->setValue("Truncate");
				preParams.getSubParamByName("Type of file")->setValue("Text");

				dummyFile->setPreInitParams(preParams);

				bool iok = false;
				dummyFile->init(&iok);

				stream << "File init: " << (iok ? "ok" : "not ok") << endl << endl;
				stream.flush();

				uint8_t writeBuffer[64] = "Mary had a little lamb...";
				size_t writtenBytes = dummyFile->writeData(writeBuffer, 10);
				stream << "Wrote " << writtenBytes << " bytes." << endl << endl;

				TConfigParam postParams = dummyFile->getPostInitParams();
				postParams.getSubParamByName("Seek to position")->setValue(0);

				dummyFile->setPostInitParams(postParams);

				uint8_t readBuffer[64] = {0, };
				size_t readBytes = dummyFile->readData(readBuffer, 63);

				stream << "Read " << readBytes << " bytes: '" << (char *) readBuffer << "'" << endl;

				postParams = dummyFile->getPostInitParams();
				stream << "Current position in file is: " << postParams.getSubParamByName("Seek to position")->getValue() << endl;
			}
		}
		pluginLoader.unload();
	}

	return 0;
}
