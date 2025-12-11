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
