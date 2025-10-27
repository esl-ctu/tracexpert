#ifndef TSCENARIOEDITORWIDGET_H
#define TSCENARIOEDITORWIDGET_H

#include <QGraphicsView>
#include <QWidget>
#include <QToolBar>

#include "qtextbrowser.h"
#include "tscenarioexecutor.h"
#include "tscenariographicsview.h"
#include "tscenario.h"
#include "tscenariocontainer.h"

QT_BEGIN_NAMESPACE
class QAction;
class QToolBox;
class QSpinBox;
class QComboBox;
class QFontComboBox;
class QButtonGroup;
class QLineEdit;
class QGraphicsTextItem;
class QFont;
class QToolButton;
class QAbstractButton;
class QGraphicsView;
QT_END_NAMESPACE

// Scenario editor code adopted from
// https://doc.qt.io/qt-6/qtwidgets-graphicsview-diagramscene-example.html
// under BSD-3-Clause license

#include "tscenarioscene.h"

/*!
 * \brief The TScenarioEditorWidget class provides a widget for editing a scenario.
 *
 * The class provides a widget for editing a scenario.
 * It has a toolbar with actions for running, stopping and saving the scenario and manipulating the items.
 * It has a toolbox with buttons for inserting different types of items.
 * It has a scene with a view for displaying the scenario.
 * It has a log view for displaying log messages.
 *
 */
class TScenarioEditorWidget : public QWidget
{

public:
    TScenarioEditorWidget(TScenarioModel * scenarioModel, TProjectModel * projectModel, QWidget * parent = nullptr);
    ~TScenarioEditorWidget();

private slots:
    void itemGroupButtonClicked(QAbstractButton *button);

    void runScenario();
    void stopScenario();
    void scenarioStopped();

    void saveScenario();
    void deleteItem();

    void pointerGroupButtonClicked();
    void pointerToolChanged(TScenarioScene::TScenarioPointerTool tool);

    void bringToFront();
    void sendToBack();

    void checkItemButton(int itemType);
    void uncheckItemButton(int itemType);

    void scaleChangedUsingMouseWheel(qreal scale);
    void sceneScaleChangedBySelection(const QString &scale);


    void lineButtonTriggered();

    void log(const QString & message, const QString & color);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QString m_originalScenarioName;
    TProjectModel * m_projectModel;
    TScenarioContainer * m_scenarioContainer;

    void createToolBox();
    void createToolBoxDrawer(const QString & title, QList<int> itemClassList);

    void createActions();
    void createToolbars();
    
    QWidget * createCellWidget(int itemClass);

    TScenario * m_runScenario;
    TScenarioScene * m_scene;
    TScenarioGraphicsView * m_view;

    QTextBrowser * m_logView;

    TScenarioExecutor * m_scenarioExecutor;
    bool m_stopRequested;
    bool m_terminateRequested;

    QAction * m_runAction;
    QAction * m_stopAction;    

    QAction * m_addAction;
    QAction * m_deleteAction;
    QAction * m_saveAction;
    QAction * m_toFrontAction;
    QAction * m_sendBackAction;

    QToolBar * m_editToolBar;
    QToolBar * m_pointerToolbar;

    QComboBox * m_sceneScaleCombo;

    QToolBox * m_toolBox;
    QButtonGroup * itemGroup;
    QButtonGroup * m_pointerTypeGroup;
};

#endif // TSCENARIOEDITORWIDGET_H
