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

#include "tprojectmigrator.h"

#include <QVector>

bool TProjectMigrator::migrate(QDomDocument & doc, QString * errorMessage)
{
    QDomElement projectElement = doc.documentElement();

    if (projectElement.tagName() != "project") {
        *errorMessage = "Unexpected project structure";
        return false;
    }

    bool ok = false;
    int version = projectElement.attribute("schemaVersion").toInt(&ok);
    if (!ok || version < 1) {
        *errorMessage = "Invalid schema version";
        return false;
    }

    const QVector<MigrationFunction> & migs = migrations();
    const int currentVersion = schemaVersion();

    if (version > currentVersion) {
        *errorMessage = QString("Project file is from a newer application version: %1 (%2)").arg(projectElement.attribute("version").arg(projectElement.attribute("revision")));
        return false;
    }

    while (version < currentVersion) {
        ok = migs[version - 1](doc);

        if (!ok) {
            *errorMessage = "Migration failed";
            return false;
        }

        version++;
        projectElement.setAttribute("schemaVersion", version);
    }

    return true;
}

int TProjectMigrator::schemaVersion()
{
    return migrations().size() + 1;
}

const QVector<TProjectMigrator::MigrationFunction> &TProjectMigrator::migrations()
{
    static const QVector<MigrationFunction> m = {
        // 1 -> 2
        /*[](QDomDocument & doc) {
            auto root = doc.documentElement();
            root.setAttribute("foo", "bar");
            return true;
        },

        // 2 -> 3
        [](QDomDocument & doc) {
            auto root = doc.documentElement();
            auto e = doc.createElement("settings");
            root.appendChild(e);
            return true;
        },

        // 3 -> 4
        [](QDomDocument & doc) {
            auto root = doc.documentElement();
            root.removeAttribute("obsolete");
            return true;
        }*/
    };

    return m;
}
