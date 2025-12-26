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

#include "thelpschemehandler.h"

THelpSchemeHandler::THelpSchemeHandler(QHelpEngine * helpEngine, QObject * parent)
    : QWebEngineUrlSchemeHandler(parent), m_helpEngine(helpEngine)
{

}

void THelpSchemeHandler::requestStarted(QWebEngineUrlRequestJob * job)
{
    const QUrl url = job->requestUrl();

    QByteArray data = m_helpEngine->fileData(url);
    if (data.isEmpty()) {
        job->fail(QWebEngineUrlRequestJob::UrlNotFound);
        return;
    }

    // Determine MIME type using QMimeDatabase
    QMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(url.path());
    QByteArray mime = mimeType.isValid() ? mimeType.name().toUtf8() : QByteArray("application/octet-stream");

    QBuffer * buffer = new QBuffer(job);
    buffer->setData(data);
    buffer->open(QIODevice::ReadOnly);

    job->reply(mime, buffer);
}
