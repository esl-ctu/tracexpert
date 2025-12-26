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

#ifndef THELPBROWSER_H
#define THELPBROWSER_H

#include <QTextBrowser>
#include <QHelpEngine>

class THelpBrowser : public QTextBrowser
{
    Q_OBJECT

public:
    THelpBrowser(QHelpEngine * helpEngine, QWidget * parent = 0);
    QVariant loadResource (int type, const QUrl & name) override;

private:
    QHelpEngine * m_helpEngine;
};

#endif // THELPBROWSER_H
