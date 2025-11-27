#include "thelpbrowser.h"

THelpBrowser::THelpBrowser(QHelpEngine * helpEngine, QWidget * parent)
    : QTextBrowser(parent), m_helpEngine(helpEngine) {}



QVariant THelpBrowser::loadResource(int type, const QUrl &name){
    if (name.scheme() == "qthelp")
        return QVariant(m_helpEngine->fileData(name));
    else
        return QTextBrowser::loadResource(type, name);
}
