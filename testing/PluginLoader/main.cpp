#include <chrono>
#include <thread>
#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include "tplugin.h"
#include <ctime>


void ReadAndWriteBack(TIODevice *device, QTextStream &stream, size_t len){
    // Add the read and write operations for the second device here
    uint8_t data[100000+1];

    if(len > 100000){
        stream << "len too big\n";
        stream.flush();
        return;
    }

    stream << "Reading..." << "\n";
    stream.flush();
    size_t read = device->readData(data, len);
    if(read == 0) {
        stream << "No data read" << "\n";
    } else {
        data[read] = 0;
        stream << (char *) data << "\n";
        /*if(data[0] != cnt){
            stream << "not sync: current " << cnt << ", recieved: " << data[0] << "\n";
        }*/
    }

    stream << "Writing..." << "\n";
    stream.flush();
    size_t writeLen;
    if(read == 0){
        uint8_t noDataText[] = "no data  \0";
        writeLen = device->writeData(noDataText, sizeof(noDataText));
    }else{
        writeLen = device->writeData(data, read);
    }
    stream << "Writed " << writeLen << " bytes\n";
    stream.flush();
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QTextStream stream(stdout);

    QDir pluginsDir(QCoreApplication::instance()->applicationDirPath());

    srand(time(0));

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

                QString portName = "COM3";
                TIODevice *selectedPort = nullptr;
                for(int i = 0; i < devices.size(); i++){
                    stream << "port name: " << devices[i]->getIODeviceName() << "\n";
                    stream << "port info: " << devices[i]->getIODeviceInfo() << "\n";
                    stream.flush();

                    if(devices[i]->getIODeviceName() == portName){
                        selectedPort = devices[i];
                        //continue;
                    }
                }


                if(!selectedPort){
                    stream << "device not found" << "\n";
                    stream.flush();
                    continue;
                }

                stream << "selected: port name: " << selectedPort->getIODeviceName() << "\n";
                stream << "selected: port info: " << selectedPort->getIODeviceInfo() << "\n";
                stream.flush();

                bool iok = false;
                selectedPort->init(&iok);
                if(!iok){
                    stream << "device cannot be inicializated" << "\n";
                    stream.flush();
                    break;
                }

                std::this_thread::sleep_for(std::chrono::seconds(3));

                std::vector<int> baudrates = {1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};
                std::vector<int> bytesizes = {8};//{5, 6, 7, 8};
                std::vector<QString> parities = {"None", "Even", "Odd", "Mark", "Space"};
                std::vector<QString> stopbits = {"One", /*"One and half",*/ "Two"};
                std::vector<QString> flow_controls = {"None", "Hardware (RTS/CTS)", "Software (XON/XOFF)"};

                //Testing one by one
                if(false){
                    int cnt = 0;

                    for (int baudrate : baudrates) {
                        for (int bytesize : bytesizes) {
                            for (const QString& parity : parities) {
                                for (const QString& stopbit : stopbits) {
                                    for (const QString& flow_control : flow_controls) {
                                        cnt++;
                                        //if(cnt<9) continue;
                                        //if(flow_control == "RTS/CTS") continue;

                                        stream << cnt << ") Trying combination: baudrate=" << baudrate << ", bytesize=" << bytesize
                                                  << ", parity=" << parity << ", stopbits=" << stopbit
                                                  << ", flow_control=" << flow_control << "\n";
                                        stream.flush();

                                        TConfigParam params = selectedPort->getPostInitParams();
                                        params.getSubParamByName("Baudrate")->setValue(baudrate);
                                        //params.getSubParamByName("Data bits")->setValue(bytesize);
                                        params.getSubParamByName("Parity")->setValue(parity);
                                        params.getSubParamByName("Stop bits")->setValue(stopbit);
                                        params.getSubParamByName("Flow control")->setValue(flow_control);
                                        params.getSubParamByName("Read timeout (ms)")->setValue("5000");
                                        selectedPort->setPostInitParams(params);


                                        ReadAndWriteBack(selectedPort, stream, 100);

                                    }
                                }
                            }
                        }
                    }
                }

                //Manual testing
                if(true){
                    // Randomly select parameters
                    int baudrate = 19200;
                    int bytesize = 8;
                    QString parity = "Even";
                    QString stopbit = "One and half";
                    QString flow_control = "None";

                    // Print the selected parameters
                    stream << "Trying combination: baudrate=" << baudrate << ", bytesize=" << bytesize
                           << ", parity=" << parity << ", stopbits=" << stopbit
                           << ", flow_control=" << flow_control << "\n";
                    stream.flush();

                    TConfigParam params = selectedPort->getPostInitParams();
                    params.getSubParamByName("Baudrate")->setValue(baudrate);
                    //params.getSubParamByName("Data bits")->setValue(bytesize);
                    params.getSubParamByName("Parity")->setValue(parity);
                    params.getSubParamByName("Stop bits")->setValue(stopbit);
                    params.getSubParamByName("Flow control")->setValue(flow_control);
                    params.getSubParamByName("Read timeout (ms)")->setValue("100000");
                    selectedPort->setPostInitParams(params);

                    ReadAndWriteBack(selectedPort, stream, 10000);
                }

                //Manual testing - random mode
                if(false){

                        // Randomly select parameters
                        int baudrate = baudrates[rand() % baudrates.size()];
                        int bytesize = bytesizes[rand() % bytesizes.size()];
                        QString parity = parities[rand() % parities.size()];
                        QString stopbit = stopbits[rand() % stopbits.size()];
                        QString flow_control = flow_controls[rand() % flow_controls.size()];
                    for(int i = 0; i<10; i++){
                        // Print the selected parameters
                        stream << "Trying combination: baudrate=" << baudrate << ", bytesize=" << bytesize
                               << ", parity=" << parity << ", stopbits=" << stopbit
                               << ", flow_control=" << flow_control << "\n";
                        stream.flush();

                        TConfigParam params = selectedPort->getPostInitParams();
                        params.getSubParamByName("Baudrate")->setValue(baudrate);
                        //params.getSubParamByName("Data bits")->setValue(bytesize);
                        params.getSubParamByName("Parity")->setValue(parity);
                        params.getSubParamByName("Stop bits")->setValue(stopbit);
                        params.getSubParamByName("Flow control")->setValue(flow_control);
                        params.getSubParamByName("Read timeout (ms)")->setValue("100000");
                        selectedPort->setPostInitParams(params);

                        ReadAndWriteBack(selectedPort, stream, 5);
                    }
                }
            }

            stream.flush();
        }
        pluginLoader.unload();
    }

    //return a.exec();
    return 0;
}
