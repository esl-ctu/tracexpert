// COPYRIGHT HEADER BEGIN
// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Adam Švehla (initial author)
// COPYRIGHT HEADER END

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


                filePlugin->addIODevice("dummy.txt", "the test file");
                filePlugin->addIODevice("dummy.txt", "the test file");

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
                size_t readBytes = dummyFile->readData(readBuffer, 10);

				stream << "Read " << readBytes << " bytes: '" << (char *) readBuffer << "'" << endl;

				postParams = dummyFile->getPostInitParams();
				stream << "Current position in file is: " << postParams.getSubParamByName("Seek to position")->getValue() << endl;

                dummyFile->deInit();

                TIODevice * dummyFile2 = filePlugin->getIODevices().at(1);

                preParams = dummyFile2->getPreInitParams();
                stream << "File path: " << preParams.getSubParamByName("File path")->getValue() << "\n\n";

                subParams = preParams.getSubParams();
                stream << "Pre-init param names: " << "\n";
                for(int i = 0; i < subParams.size(); i++){
                    stream << subParams[i].getName() << "\n";
                }
                stream << "\n\n";

                preParams.getSubParamByName("Read/Write mode")->setValue("WriteOnly");
                preParams.getSubParamByName("Write behaviour")->setValue("Truncate");
                preParams.getSubParamByName("Type of file")->setValue("Text");

                dummyFile2->setPreInitParams(preParams);

                iok = false;
                dummyFile2->init(&iok);

                stream << "File init: " << (iok ? "ok" : "not ok") << endl << endl;
                stream.flush();
			}
		}
		pluginLoader.unload();
	}

	return 0;
}
