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
