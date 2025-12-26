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
// David Pokorný (initial author)
// COPYRIGHT HEADER END

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

void printParam(QTextStream &s, TConfigParam &p, int level = 0){
    for (int var = 0; var < level; ++var) {
        s << "\t";
    }
    s << p.getName() << " = " << p.getValue() << "\n";
    for (int var = 0; var < p.getSubParams().length(); ++var) {
        printParam(s, p.getSubParams()[var], level+1);
    }
    s.flush();
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

                stream << "* Plug-in ID: '" << pluginId << "', name: '" << aPlugin->getName() << "'\n";
                stream << "    Description: '" << aPlugin->getInfo() << "'\n\n";
                stream.flush();

                if (pluginId.contains("PS6000")){
                    stream << "* Starting tests of the PS6000 plugin" << Qt::endl;

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

                                auto param1 = a[0]->getPostInitParams();
                                QString dataProbe = "Channel 3 (C)";
                                QString triggerProbe = "Channel 4 (D)";

                                param1.getSubParamByName(dataProbe)->setValue("Enabled");
                                param1.getSubParamByName(dataProbe)->getSubParamByName("Range")->setValue("-1 V .. 1 V");
                                param1.getSubParamByName(dataProbe)->getSubParamByName("Coupling/Impedance")->setValue("DC 1 MOhm");
                                //param1.getSubParamByName(dataProbe)->getSubParamByName("Analogue Offset")->setValue("-0.2");

                                param1.getSubParamByName(triggerProbe)->setValue("Enabled");
                                param1.getSubParamByName(triggerProbe)->getSubParamByName("Range")->setValue("-1 V .. 1 V");
                                //param1.getSubParamByName(triggerProbe)->getSubParamByName("Analogue Offset")->setValue("+0.1");

                                param1.getSubParamByName("Timing and acquisition")->getSubParamByName("Sampling period")->setValue("1e-6");
                                param1.getSubParamByName("Timing and acquisition")->getSubParamByName("Pre-trigger time")->setValue("0.025");
                                param1.getSubParamByName("Timing and acquisition")->getSubParamByName("Post-trigger time")->setValue("0.065");
                                param1.getSubParamByName("Timing and acquisition")->getSubParamByName("Captures per run")->setValue("20");

                                param1.getSubParamByName("Trigger")->setValue("Enabled");
                                param1.getSubParamByName("Trigger")->getSubParamByName("Source")->setValue(triggerProbe);
                                param1.getSubParamByName("Trigger")->getSubParamByName("Voltage threshold")->setValue("0.4");
                                param1.getSubParamByName("Trigger")->getSubParamByName("Auto trigger (ms)")->setValue("1000");
                                //param1.getSubParamByName("Trigger")->getSubParamByName("Direction")->setValue("Falling");

                                auto param2 = a[0]->setPostInitParams(param1);
                                stream << (param1==param2 ? "ok" : "false") << Qt::endl;
                                printParam(stream, param2);

                                size_t samplesPerTraceDownloaded, tracesDownloaded, bufferSize;

                                TScope::TSampleType stype;
                                bool overvoltage;
                                size_t expSize;

                                a[0]->run(&expSize);
                                auto buf = (uint8_t *) malloc(expSize);
                                stream << "expected size buffer: " << expSize << Qt::endl;

                                auto r = a[0]->downloadSamples(3, buf, expSize, &stype, &samplesPerTraceDownloaded, &tracesDownloaded, &overvoltage);
                                stream << "return from download: " << r << Qt::endl;

                                if(r != 0){
                                    int16_t * buf16 = (int16_t *) buf;

                                    QFile file("data");
                                    if (!file.open(QIODevice::WriteOnly)) {
                                        throw std::runtime_error("Unable to open file for writing");
                                    }
                                    QDataStream out(&file);
                                    out.setByteOrder(QDataStream::LittleEndian);
                                    for (int i = 0; i < samplesPerTraceDownloaded*tracesDownloaded; i++) {
                                        out << buf16[i];
                                    }
                                    file.close();

                                    a[0]->downloadSamples(4, buf, expSize, &stype, &samplesPerTraceDownloaded, &tracesDownloaded, &overvoltage);
                                    QFile file2("trigger");
                                    if (!file2.open(QIODevice::WriteOnly)) {
                                        throw std::runtime_error("Unable to open file for writing");
                                    }
                                    QDataStream out2(&file2);
                                    out2.setByteOrder(QDataStream::LittleEndian);
                                    for (int i = 0; i < samplesPerTraceDownloaded*tracesDownloaded; i++) {
                                        out2 << buf16[i];
                                    }
                                    file2.close();

                                    /*
                                    stream << Qt::endl;
                                    a[0]->downloadSamples(2, buf, sizeof(buf), &stype, &samplesPerTraceDownloaded, &tracesDownloaded, &overvoltage);
                                    for(int i = 0; i < samplesPerTraceDownloaded*tracesDownloaded; i++){
                                        //stream << buf2[i] << ",";
                                    }
                                    stream << Qt::endl;
                                    */
                                }


                                free(buf);

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
