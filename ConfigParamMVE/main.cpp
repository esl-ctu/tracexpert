#include <QCoreApplication>
#include <QBuffer>

#include "tconfigparam.h"


int main(int argc, char *argv[])
{

    // Vytvoreni tridy parametru
    TConfigParam rootParam("Serial console parameters", "", TConfigParam::TType::TDummy, "");
    rootParam.addSubParam(TConfigParam("Port", "COM0", TConfigParam::TType::TString, "Name of the port"));
    rootParam.addSubParam(TConfigParam("Baudrate", "9600", TConfigParam::TType::TInt, "Baudrate of the connection"));

    // Vystupni stream
    QTextStream stream(stdout);

    // Vypis tridy parametru
    stream << "Name: " << rootParam.getName() << ", value: " << rootParam.getValue() << ", hint: " << rootParam.getHint() << Qt::endl;
    // Vypis podparametru
    stream << "    Number of subparameters: " << rootParam.getSubParams().size() << Qt::endl;
    for(int i = 0; i < rootParam.getSubParams().size(); i++){
        stream << "    Name: " << rootParam.getSubParams()[i].getName() << ", value: " << rootParam.getSubParams()[i].getValue() << ", hint: " << rootParam.getSubParams()[i].getHint() << Qt::endl;
    }

    TConfigParam newParams;
    newParams = rootParam;

    // Nastaveni podparametru
    newParams.getSubParams()[0].setValue("COM2");
    //rootParam.getSubParams()[rootParam.getSubParams().indexOf("Baudrate")].setValue(115200);
    newParams.getSubParamByName("Baudrate")->setValue(115200);

    // Vypis puvodni tridy parametru
    stream << "Name: " << rootParam.getName() << ", value: " << rootParam.getValue() << ", hint: " << rootParam.getHint() << Qt::endl;
    // Vypis puvodnich podparametru
    stream << "    Number of subparameters: " << rootParam.getSubParams().size() << Qt::endl;
    for(int i = 0; i < rootParam.getSubParams().size(); i++){
        stream << "    Name: " << rootParam.getSubParams()[i].getName() << ", value: " << rootParam.getSubParams()[i].getValue() << ", hint: " << rootParam.getSubParams()[i].getHint() << Qt::endl;
    }

    // Vypis nove tridy parametru
    stream << "Name: " << newParams.getName() << ", value: " << newParams.getValue() << ", hint: " << newParams.getHint() << Qt::endl;
    // Vypis novych podparametru
    stream << "    Number of subparameters: " << newParams.getSubParams().size() << Qt::endl;
    for(int i = 0; i < newParams.getSubParams().size(); i++){
        stream << "    Name: " << newParams.getSubParams()[i].getName() << ", value: " << newParams.getSubParams()[i].getValue() << ", hint: " << newParams.getSubParams()[i].getHint() << Qt::endl;
    }

    // Serializace
    QByteArray array;
    QBuffer buf(&array);
    buf.open(QIODevice::ReadWrite);
    QDataStream dstream(&buf);

    dstream << newParams;

    // Deserializace
    buf.seek(0);
    TConfigParam deserParam;
    dstream >> deserParam;

    // Vypis deserializovane tridy parametru
    stream << "Name: " << deserParam.getName() << ", value: " << deserParam.getValue() << ", hint: " << deserParam.getHint() << Qt::endl;
    // Vypis novych podparametru
    stream << "    Number of subparameters: " << deserParam.getSubParams().size() << Qt::endl;
    for(int i = 0; i < deserParam.getSubParams().size(); i++){
        stream << "    Name: " << deserParam.getSubParams()[i].getName() << ", value: " << deserParam.getSubParams()[i].getValue() << ", hint: " << deserParam.getSubParams()[i].getHint() << Qt::endl;
    }
}
