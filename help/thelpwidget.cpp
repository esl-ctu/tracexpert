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
// Vojtěch Miškovský (initial author)

#include "thelpwidget.h"

#include <QApplication>
#include <QTabWidget>
#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QLayout>

#include "thelpbrowser.h"

THelpWidget::THelpWidget(QWidget * parent)
    : QWidget(parent)
{
    setWindowTitle(tr("Help"));

    QHelpEngine * helpEngine = new QHelpEngine(QApplication::applicationDirPath() + "/docs/docs.qhc");
    helpEngine->setupData();

    QTabWidget * helpTabWidget = new QTabWidget;
    helpTabWidget->setMaximumWidth(200);
    helpTabWidget->addTab(helpEngine->contentWidget(), "Contents");
    helpTabWidget->addTab(helpEngine->indexWidget(), "Index");

    THelpBrowser * helpBrowser = new THelpBrowser(helpEngine);

    QHBoxLayout * layout = new QHBoxLayout;
    layout->addWidget(helpTabWidget);
    layout->addWidget(helpBrowser);

    setLayout(layout);
}
