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
// Unknown (invalid) (initial author)
// Vojtěch Miškovský
// David Pokorný
// COPYRIGHT HEADER END

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

