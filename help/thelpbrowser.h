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
