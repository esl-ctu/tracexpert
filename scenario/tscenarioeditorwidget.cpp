#include "tscenarioeditorwidget.h"
#include "tscenarioexecutor.h"
#include "tscenariographicalitem.h"
#include "tscenariographicsview.h"
#include "scenario_items/tscenariobasicitems.h"
#include "scenario_items/tscenarioiodevicereaditem.h"
#include "scenario_items/tscenarioiodevicewriteitem.h"
#include "scenario_items/tscenarioscopesingleitem.h"
#include "scenario_items/tscenarioscopestartitem.h"
#include "scenario_items/tscenarioscopestopitem.h"
#include "scenario_items/tscenarioprotocolencodeitem.h"
#include "scenario_items/tscenarioloopitem.h"
#include "scenario_items/tscenariologitem.h"
#include "scenario_items/tscenariodelayitem.h"
#include "scenario_items/tscenarioconstantvalueitem.h"
#include "scenario_items/tscenariooutputfileitem.h"
#include "scenario_items/tscenariorandomstringitem.h"
#include "scenario_items/tscenariovariablereaditem.h"
#include "scenario_items/tscenariovariablewriteitem.h"
#include "scenario_items/tscenarioscriptitem.h"
#include "scenario_items/tscenarioanaldevicereaditem.h"
#include "scenario_items/tscenarioanaldevicewriteitem.h"
#include "scenario_items/tscenarioanaldeviceactionitem.h"
#include "tscenarioscene.h"
#include "../tdialog.h"

#include <QBoxLayout>
#include <QtWidgets>

TScenarioEditorWidget::TScenarioEditorWidget(TScenarioModel * scenarioModel, TProjectModel * projectModel, QWidget * parent)
    : QWidget(parent), m_projectModel(projectModel), m_scenarioContainer(projectModel->scenarioContainer())
{
    m_originalScenarioName = scenarioModel->name();

    m_scene = new TScenarioScene(projectModel, this);
    m_scene->loadScenario(scenarioModel->scenario());
    m_scene->setSceneRect(QRectF(0, 0, 5000, 5000));
    connect(m_scene, &TScenarioScene::itemInserted, this, &TScenarioEditorWidget::uncheckItemButton);
    connect(m_scene, &TScenarioScene::itemInsertStarted, this, &TScenarioEditorWidget::checkItemButton);
    connect(m_scene, &TScenarioScene::itemInsertCancelled, this, &TScenarioEditorWidget::uncheckItemButton);

    createActions();
    createToolBox();
    createToolbars();

    createToolBoxDrawer(tr("Flow blocks"),
        {   TScenarioFlowStartItem::TItemClass, TScenarioFlowEndItem::TItemClass, TScenarioFlowMergeItem::TItemClass,
            TScenarioConditionItem::TItemClass, TScenarioLoopItem::TItemClass, TScenarioDelayItem::TItemClass });

    createToolBoxDrawer(tr("Miscellaneous blocks"),
        {   TScenarioLogItem::TItemClass, TScenarioConstantValueItem::TItemClass,
            TScenarioOutputFileItem::TItemClass, TScenarioScriptItem::TItemClass,
            TScenarioVariableReadItem::TItemClass, TScenarioVariableWriteItem::TItemClass });

    createToolBoxDrawer(tr("Component blocks"),
        {   TScenarioIODeviceReadItem::TItemClass, TScenarioIODeviceWriteItem::TItemClass,
            TScenarioScopeStartItem::TItemClass, TScenarioScopeStopItem::TItemClass,
            TScenarioScopeSingleItem::TItemClass,
            TScenarioAnalDeviceReadItem::TItemClass, TScenarioAnalDeviceWriteItem::TItemClass,
            TScenarioAnalDeviceActionItem::TItemClass,
            TScenarioProtocolEncodeItem::TItemClass });

    QVBoxLayout * layout = new QVBoxLayout;

    QHBoxLayout * toolBarLayout = new QHBoxLayout;
    toolBarLayout->addWidget(m_editToolBar);
    toolBarLayout->addWidget(m_pointerToolbar);
    layout->addLayout(toolBarLayout);

    QHBoxLayout * lowerLayout = new QHBoxLayout;
    lowerLayout->addWidget(m_toolBox);

    m_view = new TScenarioGraphicsView(m_scene);
    m_view->setMouseTracking(true);
    m_view->setDragMode(QGraphicsView::RubberBandDrag);
    m_view->setFocusPolicy(Qt::StrongFocus);
    m_view->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_view->centerOn(0, 0);
    lowerLayout->addWidget(m_view);

    connect(m_view, &TScenarioGraphicsView::scaleChangedUsingMouseWheel,
            this, &TScenarioEditorWidget::scaleChangedUsingMouseWheel);

    connect(m_scene, &TScenarioScene::pointerToolChanged,
            this, &TScenarioEditorWidget::pointerToolChanged);

    layout->addLayout(lowerLayout);

    m_logView = new QTextBrowser;
    m_logView->setReadOnly(true);
    m_logView->setMinimumHeight(150);

    QVBoxLayout * logViewBoxLayout = new QVBoxLayout;
    logViewBoxLayout->addWidget(m_logView);

    QGroupBox * logViewBox = new QGroupBox;
    logViewBox->setTitle(tr("Scenario run log"));
    logViewBox->setLayout(logViewBoxLayout);
    layout->addWidget(logViewBox);

    setLayout(layout);

    m_scenarioExecutor = new TScenarioExecutor(m_projectModel);
    connect(m_scenarioExecutor, &TScenarioExecutor::scenarioExecutionFinished, this, &TScenarioEditorWidget::scenarioStopped);
    connect(m_scenarioExecutor, &TScenarioExecutor::log, this, &TScenarioEditorWidget::log);
}

TScenarioEditorWidget::~TScenarioEditorWidget() { }

void TScenarioEditorWidget::log(const QString & message, const QString & color) {
    m_logView->append(QString("<span style=\"color:%1\">%2</span>").arg(color, message));
}

void TScenarioEditorWidget::closeEvent(QCloseEvent *event) {
    event->ignore();

    if(TDialog::closeConfirmation(this)) {
        event->accept();
    }
}

void TScenarioEditorWidget::saveScenario() {
    int idx = m_scenarioContainer->getIndexByName(m_originalScenarioName);

    if(idx >= 0) {
        m_scenarioContainer->update(idx, m_scene->copyScenario());
    }
    else {
        m_scenarioContainer->add(m_scene->copyScenario());
    }
}

void TScenarioEditorWidget::runScenario() {
    m_runAction->setEnabled(false);
    m_stopAction->setEnabled(true);
    m_stopRequested = false;
    m_terminateRequested = false;

    m_logView->clear();

    m_scenarioExecutor->start(m_scene->scenario());
}

void TScenarioEditorWidget::stopScenario() {
    if(!m_stopRequested) {
        m_stopRequested = true;

        m_scenarioExecutor->stop();
    }
    else if(!m_terminateRequested && TDialog::scenarioTerminationConfirmation(this)) {
        m_terminateRequested = true;
        m_stopAction->setEnabled(false);

        m_scenarioExecutor->terminate();
    }
}

void TScenarioEditorWidget::scenarioStopped() {
    m_runAction->setEnabled(true);
    m_stopAction->setEnabled(false);
}

void TScenarioEditorWidget::itemGroupButtonClicked(QAbstractButton * button)
{
    int insertedItemClass = itemGroup->id(button);
    m_scene->setInsertItemMode(
        TScenarioGraphicalItem::createScenarioGraphicalItem(
            TScenarioItem::createScenarioItemByClass(insertedItemClass)
        )
    );
}

void TScenarioEditorWidget::pointerGroupButtonClicked() {
    if(m_pointerTypeGroup->checkedId() != m_scene->pointerTool()) {
        m_scene->setPointerTool((TScenarioScene::TScenarioPointerTool)m_pointerTypeGroup->checkedId());
    }
}

void TScenarioEditorWidget::pointerToolChanged(TScenarioScene::TScenarioPointerTool tool) {
    switch(m_scene->pointerTool()) {
        case TScenarioScene::MouseDrag:
            m_pointerTypeGroup->button(TScenarioScene::MouseDrag)->setChecked(true);
            m_view->setDragMode(QGraphicsView::ScrollHandDrag);
            break;
        case TScenarioScene::MousePointer:
            m_pointerTypeGroup->button(TScenarioScene::MousePointer)->setChecked(true);
            m_view->setDragMode(QGraphicsView::RubberBandDrag);
            break;
        case TScenarioScene::InsertLine:
            m_pointerTypeGroup->button(TScenarioScene::InsertLine)->setChecked(true);
            m_view->setDragMode(QGraphicsView::NoDrag);
            break;
        default:
            m_pointerTypeGroup->button(TScenarioScene::MousePointer)->setChecked(true);
            m_view->setDragMode(QGraphicsView::NoDrag);
            break;
    }
}

void TScenarioEditorWidget::keyPressEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Shift) {
        m_pointerTypeGroup->button(TScenarioScene::MouseDrag)->click();
    }

    QWidget::keyPressEvent(event);
}

void TScenarioEditorWidget::keyReleaseEvent(QKeyEvent *event) {
    if(event->key() == Qt::Key_Shift) {
        m_pointerTypeGroup->button(TScenarioScene::MousePointer)->click();
    }

    QWidget::keyPressEvent(event);
}

void TScenarioEditorWidget::checkItemButton(int itemClass)
{
    QAbstractButton * pressedButton = itemGroup->button(itemClass);
    if(pressedButton) {
        pressedButton->setChecked(true);
    }
}

void TScenarioEditorWidget::uncheckItemButton(int itemClass)
{
    QAbstractButton * pressedButton = itemGroup->button(itemClass);
    if(pressedButton) {
        pressedButton->setChecked(false);
    }
}

void TScenarioEditorWidget::scaleChangedUsingMouseWheel(qreal scale)
{
    m_sceneScaleCombo->setCurrentIndex(-1);
}


void TScenarioEditorWidget::sceneScaleChangedBySelection(const QString &scale)
{
    double newScale = scale.left(scale.indexOf(tr("%"))).toDouble() / 100.0;

    if(newScale != 0) {
        QTransform oldMatrix = m_view->transform();
        m_view->resetTransform();
        m_view->translate(oldMatrix.dx(), oldMatrix.dy());
        m_view->scale(newScale, newScale);
    }
}

void TScenarioEditorWidget::createToolBox()
{
    itemGroup = new QButtonGroup(this);
    itemGroup->setExclusive(false);
    connect(itemGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &TScenarioEditorWidget::itemGroupButtonClicked);

    m_toolBox = new QToolBox;
    m_toolBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored));
}

void TScenarioEditorWidget::createToolBoxDrawer(const QString & title, QList<int> itemClassList)
{
    QGridLayout * layout = new QGridLayout;
    // hack: row has to be higher than actual count of rows for no stretching
    layout->setRowStretch(999, 10);
    //layout->setColumnStretch(2, 10);

    for(int i = 0; i < itemClassList.count(); i++) {
        QWidget * widget = createCellWidget(itemClassList[i]);

        if(!widget) {
            continue;
        }

        layout->addWidget(widget, i/2, i%2);
    }

    QWidget * itemWidget = new QWidget;
    itemWidget->setLayout(layout);

    int width = itemWidget->sizeHint().width();
    if(width > m_toolBox->minimumWidth()) {
        m_toolBox->setMinimumWidth(width);
    }

    m_toolBox->addItem(itemWidget, title);
}

void TScenarioEditorWidget::createActions() {
    m_toFrontAction = new QAction(QIcon(":/icons/bringtofront.png"),
                                tr("Bring to &Front"), this);
    m_toFrontAction->setShortcut(tr("Ctrl+F"));
    m_toFrontAction->setStatusTip(tr("Bring selected item(s) to front"));
    connect(m_toFrontAction, &QAction::triggered, this->m_scene, &TScenarioScene::bringSelectedToFront);

    m_sendBackAction = new QAction(QIcon(":/icons/sendtoback.png"), tr("Send to &Back"), this);
    m_sendBackAction->setShortcut(tr("Ctrl+T"));
    m_sendBackAction->setStatusTip(tr("Send selected item(s) to back"));
    connect(m_sendBackAction, &QAction::triggered, this->m_scene, &TScenarioScene::sendSelectedToBack);

    m_deleteAction = new QAction(QIcon(":/icons/delete.png"), tr("&Delete"), this);
    m_deleteAction->setShortcut(tr("Delete"));
    m_deleteAction->setStatusTip(tr("Delete selected item(s) from diagram"));
    connect(m_deleteAction, &QAction::triggered, this->m_scene, &TScenarioScene::removeSelectedItems);

    m_saveAction = new QAction(QIcon(":/icons/save.png"), tr("&Save"), this);
    m_saveAction->setShortcut(tr("Ctrl+S"));
    m_saveAction->setStatusTip(tr("Save scenario diagram"));
    connect(m_saveAction, &QAction::triggered, this, &TScenarioEditorWidget::saveScenario);

    m_runAction = new QAction(QIcon(":/icons/play.png"), tr("&Run"), this);
    m_runAction->setStatusTip(tr("Run scenario"));
    connect(m_runAction, &QAction::triggered, this, &TScenarioEditorWidget::runScenario);

    m_stopAction = new QAction(QIcon(":/icons/stop.png"), tr("&Stop"), this);
    m_stopAction->setStatusTip(tr("Stop running scenario"));
    connect(m_stopAction, &QAction::triggered, this, &TScenarioEditorWidget::stopScenario);
    m_stopAction->setEnabled(false);
}

void TScenarioEditorWidget::createToolbars() {
    m_editToolBar = new QToolBar(tr("Edit"));
    m_editToolBar->addAction(m_runAction);
    m_editToolBar->addAction(m_stopAction);
    m_editToolBar->addAction(m_saveAction);
    m_editToolBar->addAction(m_toFrontAction);
    m_editToolBar->addAction(m_sendBackAction);
    m_editToolBar->addAction(m_deleteAction);

    QToolButton *dragPointerButton = new QToolButton;
    dragPointerButton->setToolTip(tr("Drag scene (toggle using Shift)"));
    dragPointerButton->setCheckable(true);
    dragPointerButton->setChecked(true);
    dragPointerButton->setIcon(QIcon(":/icons/dragpointer.png"));

    QToolButton *pointerButton = new QToolButton;
    pointerButton->setToolTip(tr("Move/select items"));
    pointerButton->setCheckable(true);
    pointerButton->setChecked(true);
    pointerButton->setIcon(QIcon(":/icons/pointer.png"));

    QToolButton *linePointerButton = new QToolButton;
    linePointerButton->setToolTip(tr("Create connection"));
    linePointerButton->setCheckable(true);
    linePointerButton->setIcon(QIcon(":/icons/linepointer.png"));

    m_pointerTypeGroup = new QButtonGroup(this);
    m_pointerTypeGroup->addButton(dragPointerButton, TScenarioScene::MouseDrag);
    m_pointerTypeGroup->addButton(pointerButton, TScenarioScene::MousePointer);
    m_pointerTypeGroup->addButton(linePointerButton, TScenarioScene::InsertLine);
    connect(m_pointerTypeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),
            this, &TScenarioEditorWidget::pointerGroupButtonClicked);

    m_sceneScaleCombo = new QComboBox;    
    QStringList scales;
    scales << tr("50%") << tr("75%") << tr("100%") << tr("125%") << tr("150%");
    m_sceneScaleCombo->addItems(scales);    
    m_sceneScaleCombo->setCurrentIndex(2);
    m_sceneScaleCombo->setToolTip(tr("Zoom"));
    connect(m_sceneScaleCombo, &QComboBox::currentTextChanged,
            this, &TScenarioEditorWidget::sceneScaleChangedBySelection);

    m_pointerToolbar = new QToolBar(tr("Pointer type"));
    m_pointerToolbar->addWidget(dragPointerButton);
    m_pointerToolbar->addWidget(pointerButton);
    m_pointerToolbar->addWidget(linePointerButton);
    m_pointerToolbar->addWidget(m_sceneScaleCombo);
}

QWidget * TScenarioEditorWidget::createCellWidget(int itemClass) {

    TScenarioItem * scenarioItem = TScenarioItem::createScenarioItemByClass(itemClass);

    if(!scenarioItem) {
        return nullptr;
    }

    TScenarioGraphicalItem * graphicalItem = TScenarioGraphicalItem::createScenarioGraphicalItem(scenarioItem);

    QIcon icon(graphicalItem->image());

    QToolButton * button = new QToolButton;
    button->setIcon(icon);
    button->setIconSize(QSize(50, 50));
    button->setCheckable(true);
    itemGroup->addButton(button, itemClass);

    QString labelText = graphicalItem->getScenarioItem()->getName();
    if(labelText.contains(':')) {
        labelText.prepend("<b>");
        labelText.replace(": ", "</b><br>");
    }

    QLabel * label = new QLabel(labelText);
    label->setAlignment(Qt::AlignCenter);

    QGridLayout * layout = new QGridLayout;
    layout->addWidget(button, 0, 0, Qt::AlignHCenter);
    layout->addWidget(label, 1, 0, Qt::AlignHCenter);

    QWidget * widget = new QWidget;
    widget->setLayout(layout);
    widget->setFixedWidth(120);

    delete graphicalItem;

    return widget;
}

