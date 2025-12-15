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
