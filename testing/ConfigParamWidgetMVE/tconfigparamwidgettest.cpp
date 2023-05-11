#include "tconfigparamwidgettest.h"

#include <QPushButton>
#include <QGridLayout>

#include "tconfigparamwidget.h"

TConfigParamWidgetTest::TConfigParamWidgetTest(QWidget *parent)
    : QWidget{parent}
{
    TConfigParam rootParam("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", false);
    rootParam.setState(TConfigParam::TState::TWarning, "Something went a little bit wrong");
    rootParam.addSubParam(TConfigParam("Sample string parameter", "", TConfigParam::TType::TString, "Hint of sample string parameter", false));
    rootParam.addSubParam(TConfigParam("Sample integer", "555", TConfigParam::TType::TInt, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned integer", "", TConfigParam::TType::TUInt, "", false));
    rootParam.addSubParam(TConfigParam("Sample short", "", TConfigParam::TType::TShort, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned short", "", TConfigParam::TType::TUShort, "", false));
    rootParam.addSubParam(TConfigParam("Sample long long", "", TConfigParam::TType::TLongLong, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned long long", "", TConfigParam::TType::TULongLong, "", false));
    rootParam.addSubParam(TConfigParam("Sample double", "", TConfigParam::TType::TReal, "", false));
    TConfigParam subParam(TConfigParam("Sample bool", "", TConfigParam::TType::TBool, "", false));
    subParam.setState(TConfigParam::TState::TError, "Something went terribly wrong");
    subParam.addSubParam(TConfigParam("Sample subsubparam 1", "", TConfigParam::TType::TString, "", false));
    subParam.addSubParam(TConfigParam("Sample subsubparam 2", "", TConfigParam::TType::TString, "", false));
    subParam.addSubParam(TConfigParam("Sample read-only subsubparam 3", "", TConfigParam::TType::TString, "", true));
    rootParam.addSubParam(subParam);
    TConfigParam enumParam("Sample enum", "", TConfigParam::TType::TEnum, "", false);
    enumParam.setState(TConfigParam::TState::TInfo, "Something needs to be said");
    enumParam.addEnumValue("Sample enum value 1");
    enumParam.addEnumValue("Sample enum value 2");
    enumParam.addEnumValue("Sample enum value 3");
    enumParam.addEnumValue("Sample enum value 4");
    enumParam.addEnumValue("Sample enum value 5");
    rootParam.addSubParam(enumParam);
    rootParam.addSubParam(TConfigParam("Sample time edit", "0.0000000000005", TConfigParam::TType::TTime, "", false));
    rootParam.addSubParam(TConfigParam("Sample file name edit", "", TConfigParam::TType::TFileName, "", false));

    TConfigParamWidget * configParamWidget = new TConfigParamWidget(rootParam, this);

    QPushButton * applyButton = new QPushButton("Apply");

    connect(applyButton, SIGNAL(clicked()), configParamWidget, SLOT(param()));

    QGridLayout * layout = new QGridLayout(this);

    layout->addWidget(configParamWidget);
    layout->addWidget(applyButton);

    setLayout(layout);
}

