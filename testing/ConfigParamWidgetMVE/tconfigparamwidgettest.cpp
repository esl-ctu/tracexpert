#include "tconfigparamwidgettest.h"

#include <QPushButton>
#include <QGridLayout>

#include "tconfigparamwidget.h"

TConfigParamWidgetTest::TConfigParamWidgetTest(TConfigParam configParam, QWidget *parent)
    : QWidget{parent}
{

    TConfigParam rootParam("Sample dummy parameter", "", TConfigParam::TType::TDummy, "Hint of sample dummy parameter", false);
    rootParam.setState(TConfigParam::TState::TWarning, "Something went a little bit wrong");
    rootParam.addSubParam(TConfigParam("Sample string parameter", "po", TConfigParam::TType::TString, "Hint of sample string parameter", false));
    rootParam.addSubParam(TConfigParam("Sample integer", "555", TConfigParam::TType::TInt, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned integer", "", TConfigParam::TType::TUInt, "", false));
    rootParam.addSubParam(TConfigParam("Sample short", "55555555", TConfigParam::TType::TShort, "", false));
    rootParam.addSubParam(TConfigParam("Sample unsigned short", "", TConfigParam::TType::TUShort, "", false));

    configParamWidget = new TConfigParamWidget(rootParam, this);
    configParamWidget->setParam(configParam);
    QPushButton * applyButton = new QPushButton("Apply");

    connect(applyButton, SIGNAL(clicked()), configParamWidget, SLOT(param()));

    QGridLayout * layout = new QGridLayout(this);

    layout->addWidget(configParamWidget);
    layout->addWidget(applyButton);

    setLayout(layout);
}

