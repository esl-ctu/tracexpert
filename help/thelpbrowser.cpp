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

#include "thelpbrowser.h"

#include <QHelpContentWidget>
#include <QHelpIndexWidget>
#include <QHelpLink>

THelpBrowser::THelpBrowser(QHelpEngine * helpEngine, QWidget * parent)
    : QTextBrowser(parent), m_helpEngine(helpEngine)
{
    setSource(QUrl("qthelp://org.cvut.fit.tracexpert/docs/README.html"));
    connect(helpEngine->contentWidget(), &QHelpContentWidget::linkActivated, this, [=](const QUrl & source) { setSource(source); });
    connect(helpEngine->indexWidget(), &QHelpIndexWidget::documentActivated, this, [=](const QHelpLink & document) { setSource(document.url); });
}

QVariant THelpBrowser::loadResource(int type, const QUrl &name){
    if (name.scheme() == "qthelp")
        return QVariant(m_helpEngine->fileData(name));
    else
        return QTextBrowser::loadResource(type, name);
}
