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
// Petr Socha (initial author)
// COPYRIGHT HEADER END

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
