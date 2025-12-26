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
#include <QBuffer>

#include "tconfigparam.h"


int main(int argc, char *argv[])
{

    // Vytvoreni tridy parametru
    TConfigParam rootParam("Serial console parameters", "", TConfigParam::TType::TDummy, "", false);
    rootParam.addSubParam(TConfigParam("Port", "COM0", TConfigParam::TType::TString, "Name of the port", false));
    rootParam.addSubParam(TConfigParam("Baudrate", "9600", TConfigParam::TType::TInt, "Baudrate of the connection", false));

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
//    newParams.getSubParamByName("Baudrate")->setValue(115200);
    TConfigParam * subparam = newParams.getSubParamByName("Baudrate");
    if(subparam == nullptr){
        stream << "Subparametr nenalezen" << Qt::endl;
    } else {
        subparam->setValue(115200);
    }

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
