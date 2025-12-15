#ifndef THELPSCHEMEHANDLER_H
#define THELPSCHEMEHANDLER_H

#include <QHelpEngine>
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>
#include <QMimeDatabase>
#include <QBuffer>

class THelpSchemeHandler : public QWebEngineUrlSchemeHandler
{
public:
    THelpSchemeHandler(QHelpEngine * helpEngine, QObject * parent = nullptr);

    void requestStarted(QWebEngineUrlRequestJob * job) override;

private:
    QHelpEngine * m_helpEngine;
};

#endif // THELPSCHEMEHANDLER_H
