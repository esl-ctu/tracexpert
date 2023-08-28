#include <QApplication>
#include <iostream>
#include <ostream>

#include "tconfigparam.h"
#include "tconfigparamwidgettest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    TConfigParam p1("Sample dummy parameter", "ahoj", TConfigParam::TType::TString, "Hint of sample dummy parameter", false);
    TConfigParam p2 = p1;
    std::cout << "A" << (p2.getValue()).toStdString() << std::endl;
    std::cout << "B" << (p1.getValue()).toStdString() << std::endl;



    TConfigParam rootParam("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", false);
    rootParam.setState(TConfigParam::TState::TWarning, "Something went a little bit wrong");
    rootParam.addSubParam(TConfigParam("Sample string parameter", "po", TConfigParam::TType::TString, "Hint of sample string parameter", false));
    rootParam.addSubParam(TConfigParam("Sample integer", "555", TConfigParam::TType::TInt, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned integer", "", TConfigParam::TType::TUInt, "", false));
    rootParam.addSubParam(TConfigParam("Sample short", "55555555", TConfigParam::TType::TShort, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned short", "", TConfigParam::TType::TUShort, "", false));
    rootParam.addSubParam(TConfigParam("Sample long long", "", TConfigParam::TType::TLongLong, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned long long", "", TConfigParam::TType::TULongLong, "", false));
    rootParam.addSubParam(TConfigParam("Sample double", "", TConfigParam::TType::TReal, "", false));
    TConfigParam subParam(TConfigParam("Sample bool", "", TConfigParam::TType::TBool, "", true));
    //subParam.setState(TConfigParam::TState::TError, "Something went terribly wrong");
    subParam.addSubParam(TConfigParam("Sample subsubparam 1", "", TConfigParam::TType::TString, "", false));
    subParam.addSubParam(TConfigParam("Sample subsubparam 2", "", TConfigParam::TType::TLongLong, "", false));
    subParam.addSubParam(TConfigParam("Sample read-only subsubparam 3", "", TConfigParam::TType::TString, "", true));
    rootParam.addSubParam(subParam);
    TConfigParam enumParam("Sample enum", "Sample enum value 20", TConfigParam::TType::TEnum, "", false);
    enumParam.setState(TConfigParam::TState::TInfo, "Something needs to be said");
    enumParam.addEnumValue("Sample enum value 1");
    enumParam.addEnumValue("Sample enum value 2");
    enumParam.addEnumValue("Sample enum value 3");
    enumParam.addEnumValue("Sample enum value 4");
    enumParam.addEnumValue("Sample enum value 5");
    enumParam.addEnumValue("Sample enum value 5");
    rootParam.addSubParam(enumParam);
    rootParam.addSubParam(TConfigParam("Sample time edit", "0.0000000000005", TConfigParam::TType::TTime, "", false));
    rootParam.addSubParam(TConfigParam("Sample file name edit", "", TConfigParam::TType::TFileName, "", true));

    TConfigParam subParam6("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", false);
    subParam6.addSubParam(TConfigParam("Sample parameter", "po", TConfigParam::TType::TUShort, "Hint of sample string parameter", false));

    TConfigParam subParam5("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", false);
    subParam5.addSubParam(TConfigParam("Sample parameter", "po", TConfigParam::TType::TUShort, "Hint of sample string parameter", false));

    TConfigParam subParam4("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", true);
    subParam4.addSubParam(TConfigParam("Sample parameter", "po", TConfigParam::TType::TUShort, "Hint of sample string parameter", false));

    TConfigParam subParam3("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", false);
    subParam3.addSubParam(TConfigParam("Sample parameter", "po", TConfigParam::TType::TUShort, "Hint of sample string parameter", false));

    TConfigParam subParam2("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", false);
    subParam2.addSubParam(TConfigParam("Sample string parameter", "po", TConfigParam::TType::TString, "Hint of sample string parameter", false));
    subParam2.addSubParam(TConfigParam("Sample integer", "555", TConfigParam::TType::TInt, "", false));
    subParam2.addSubParam(TConfigParam("Sample unsigned integer", "", TConfigParam::TType::TUInt, "", false));
    subParam2.addSubParam(TConfigParam("Sample short", "55555555", TConfigParam::TType::TShort, "", false));
    subParam2.addSubParam(TConfigParam("Sample unsigned short", "", TConfigParam::TType::TUShort, "", false));
    subParam2.addSubParam(TConfigParam("Sample long long", "", TConfigParam::TType::TLongLong, "", false));
    subParam2.addSubParam(TConfigParam("Sample unsigned long long", "", TConfigParam::TType::TULongLong, "", false));
    subParam2.addSubParam(TConfigParam("Sample double", "", TConfigParam::TType::TReal, "", false));


    subParam5.addSubParam(subParam6);
    subParam4.addSubParam(subParam5);
    subParam3.addSubParam(subParam4);
    subParam2.addSubParam(subParam3);
    rootParam.addSubParam(subParam2);

    while(true){
        TConfigParamWidgetTest w1(rootParam);
        w1.show();
        a.exec();
        rootParam = w1.configParamWidget->param();
    }

    return 0;
}
