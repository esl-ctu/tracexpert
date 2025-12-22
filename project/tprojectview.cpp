#include "tprojectview.h"

#include <QMenu>
#include <QApplication>

#include "../tdialog.h"
#include "../tdevicewizard.h"

TProjectView::TProjectView(QToolBar * toolbar, QWidget * parent)
    : QTreeView(parent), m_toolbar(toolbar)
{
    createActions();

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &TProjectView::customContextMenuRequested, this, &TProjectView::showContextMenu);
    connect(this, &TProjectView::doubleClicked, this, &TProjectView::runDefaultAction);
}

void TProjectView::setModel(QAbstractItemModel * newModel)
{
    QTreeView::setModel(newModel);

    connect(model(), &TProjectModel::dataChanged, this, &TProjectView::refreshActions);
}

void TProjectView::createActions()
{
    m_initComponentAction = new QAction(tr("Initialize"), this);
    m_initComponentAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));
    connect(m_initComponentAction, &QAction::triggered, this, &TProjectView::initComponent);
    m_deinitComponentAction = new QAction(tr("Deinitialize"), this);
    m_deinitComponentAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton));
    connect(m_deinitComponentAction, &QAction::triggered, this, &TProjectView::deinitComponent);
    m_showComponentSettingsAction = new QAction(tr("Settings"), this);
    m_showComponentSettingsAction->setIcon(QPixmap(":/icons/settings.png"));
    connect(m_showComponentSettingsAction, &QAction::triggered, this, &TProjectView::showComponentSettings);
    m_openDeviceAction = new QAction(tr("Open device"), this);
    m_openDeviceAction->setIcon(QPixmap(":/icons/add-open-device.png"));
    connect(m_openDeviceAction, &QAction::triggered, this, &TProjectView::openDevice);

    m_addIODeviceAction = new QAction(tr("Add IO device"), this);
    m_addIODeviceAction->setIcon(QPixmap(":/icons/add-iodevice.png"));
    connect(m_addIODeviceAction, &QAction::triggered, this, &TProjectView::addIODevice);
    m_addScopeAction = new QAction(tr("Add oscilloscope"), this);
    m_addScopeAction->setIcon(QPixmap(":/icons/add-oscilloscope.png"));
    connect(m_addScopeAction, &QAction::triggered, this, &TProjectView::addScope);
    m_addAnalDeviceAction = new QAction(tr("Add analytical device"), this);
    m_addAnalDeviceAction->setIcon(QPixmap(":/icons/add-analytical.png"));
    connect(m_addAnalDeviceAction, &QAction::triggered, this, &TProjectView::addAnalDevice);

    m_initIODeviceAction = new QAction(tr("Initialize"), this);
    m_initIODeviceAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));
    connect(m_initIODeviceAction, &QAction::triggered, this, &TProjectView::initIODevice);
    m_deinitIODeviceAction = new QAction(tr("Deinitialize"), this);
    m_deinitIODeviceAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton));
    connect(m_deinitIODeviceAction, &QAction::triggered, this, &TProjectView::deinitIODevice);
    m_showIODeviceAction = new QAction(tr("Show"), this);
    m_showIODeviceAction->setIcon(QPixmap(":/icons/show.png"));
    connect(m_showIODeviceAction, &QAction::triggered, this, &TProjectView::showIODevice);
    m_removeIODeviceAction = new QAction(tr("Remove"), this);
    m_removeIODeviceAction->setIcon(QPixmap(":/icons/delete.png"));;
    connect(m_removeIODeviceAction, &QAction::triggered, this, &TProjectView::removeIODevice);

    m_initScopeAction = new QAction(tr("Initialize"), this);
    m_initScopeAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));
    connect(m_initScopeAction, &QAction::triggered, this, &TProjectView::initScope);
    m_deinitScopeAction = new QAction(tr("Deinitialize"), this);
    m_deinitScopeAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton));
    connect(m_deinitScopeAction, &QAction::triggered, this, &TProjectView::deinitScope);
    m_showScopeAction = new QAction(tr("Show"), this);
    m_showScopeAction->setIcon(QPixmap(":/icons/show.png"));
    connect(m_showScopeAction, &QAction::triggered, this, &TProjectView::showScope);
    m_removeScopeAction = new QAction(tr("Remove"), this);
    m_removeScopeAction->setIcon(QPixmap(":/icons/delete.png"));
    connect(m_removeScopeAction, &QAction::triggered, this, &TProjectView::removeScope);

    m_initAnalDeviceAction = new QAction(tr("Initialize"), this);
    m_initAnalDeviceAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogYesButton));
    connect(m_initAnalDeviceAction, &QAction::triggered, this, &TProjectView::initAnalDevice);
    m_deinitAnalDeviceAction = new QAction(tr("Deinitialize"), this);
    m_deinitAnalDeviceAction->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogNoButton));
    connect(m_deinitAnalDeviceAction, &QAction::triggered, this, &TProjectView::deinitAnalDevice);
    m_showAnalDeviceAction = new QAction(tr("Show"), this);
    m_showAnalDeviceAction->setIcon(QPixmap(":/icons/show.png"));
    connect(m_showAnalDeviceAction, &QAction::triggered, this, &TProjectView::showAnalDevice);
    m_removeAnalDeviceAction = new QAction(tr("Remove"), this);
    m_removeAnalDeviceAction->setIcon(QPixmap(":/icons/delete.png"));
    connect(m_removeAnalDeviceAction, &QAction::triggered, this, &TProjectView::removeAnalDevice);

    m_showInfoAction = new QAction(tr("Info"), this);
    m_showInfoAction->setIcon(QPixmap(":/icons/info.png"));
    connect(m_showInfoAction, &QAction::triggered, this, &TProjectView::showInfo);

    m_openProtocolManagerAction = new QAction(tr("Open Protocol Manager"), this);
    m_openProtocolManagerAction->setIcon(QPixmap(":/icons/protocol-manager.png"));
    connect(m_openProtocolManagerAction, &QAction::triggered, this, &TProjectView::showProtocolManager);
    m_editProtocolAction = new QAction(tr("Edit"), this);
    m_editProtocolAction->setIcon(QPixmap(":/icons/show.png"));
    connect(m_editProtocolAction, &QAction::triggered, this, &TProjectView::openProtocolEditor);

    m_openScenarioManagerAction = new QAction(tr("Open Scenario Manager"), this);
    m_openScenarioManagerAction->setIcon(QPixmap(":/icons/scenario-manager.png"));
    connect(m_openScenarioManagerAction, &QAction::triggered, this, &TProjectView::showScenarioManager);
    m_editScenarioAction = new QAction(tr("Edit"), this);
    m_editScenarioAction->setIcon(QPixmap(":/icons/show.png"));
    connect(m_editScenarioAction, &QAction::triggered, this, &TProjectView::openScenarioEditor);
}

void TProjectView::refreshActions()
{
    TProjectItem * item = nullptr;

    if (!selectedIndexes().isEmpty())
        item = static_cast<TProjectItem *>(selectedIndexes().constFirst().internalPointer());

    refreshActionsForItem(item);
}

void TProjectView::refreshActionsForItem(TProjectItem * item)
{
    m_currentActions.clear();
    m_currentDefaultAction = nullptr;

    m_component = nullptr;
    m_IODevice = nullptr;
    m_scope = nullptr;
    m_analDevice = nullptr;
    m_unit = nullptr;
    m_protocol = nullptr;
    m_scenario = nullptr;

    if ((m_component = dynamic_cast<TComponentModel *>(item))) {
        m_unit = m_component;

        m_initComponentAction->setDisabled(m_component->status() != TProjectItem::Uninitialized);
        m_currentActions.append(m_initComponentAction);
        m_deinitComponentAction->setDisabled(m_component->status() != TProjectItem::Initialized);
        m_currentActions.append(m_deinitComponentAction);
        m_showComponentSettingsAction->setDisabled(m_component->status() != TProjectItem::Initialized);
        m_currentActions.append(m_showComponentSettingsAction);
        m_currentActions.append(m_openDeviceAction);
        m_addIODeviceAction->setDisabled(m_component->status() != TProjectItem::Initialized || !m_component->canAddIODevice());
        m_currentActions.append(m_addIODeviceAction);
        m_addScopeAction->setDisabled(m_component->status() != TProjectItem::Initialized || !m_component->canAddScope());
        m_currentActions.append(m_addScopeAction);
        m_addAnalDeviceAction->setDisabled(m_component->status() != TProjectItem::Initialized || !m_component->canAddAnalDevice());
        m_currentActions.append(m_addAnalDeviceAction);

        m_currentDefaultAction = chooseDefaultAction(m_component);
    }
    else if ((m_IODevice = dynamic_cast<TIODeviceModel *>(item))) {
        m_unit = m_IODevice;

        m_initIODeviceAction->setDisabled(m_IODevice->status() != TProjectItem::Uninitialized);
        m_currentActions.append(m_initIODeviceAction);
        m_deinitIODeviceAction->setDisabled(m_IODevice->status() != TProjectItem::Initialized);
        m_currentActions.append(m_deinitIODeviceAction);
        m_showIODeviceAction->setDisabled(m_IODevice->status() != TProjectItem::Initialized);
        m_currentActions.append(m_showIODeviceAction);
        m_removeIODeviceAction->setDisabled(!m_IODevice->isManual() && m_IODevice->isAvailable());
        m_currentActions.append(m_removeIODeviceAction);

        m_currentDefaultAction = chooseDefaultAction(m_IODevice);
    }
    else if ((m_scope = dynamic_cast<TScopeModel *>(item))) {
        m_unit = m_scope;

        m_initScopeAction->setDisabled(m_scope->status() != TProjectItem::Uninitialized);
        m_currentActions.append(m_initScopeAction);
        m_deinitScopeAction->setDisabled(m_scope->status() != TProjectItem::Initialized);
        m_currentActions.append(m_deinitScopeAction);
        m_showScopeAction->setDisabled(m_scope->status() != TProjectItem::Initialized);
        m_currentActions.append(m_showScopeAction);
        m_removeScopeAction->setDisabled(!m_scope->isManual() && m_scope->isAvailable());
        m_currentActions.append(m_removeScopeAction);

        m_currentDefaultAction = chooseDefaultAction(m_scope);
    }
    else if ((m_analDevice = dynamic_cast<TAnalDeviceModel *>(item))) {
        m_unit = m_analDevice;

        m_initAnalDeviceAction->setDisabled(m_analDevice->status() != TProjectItem::Uninitialized);
        m_currentActions.append(m_initAnalDeviceAction);
        m_deinitAnalDeviceAction->setDisabled(m_analDevice->status() != TProjectItem::Initialized);
        m_currentActions.append(m_deinitAnalDeviceAction);
        m_showAnalDeviceAction->setDisabled(m_analDevice->status() != TProjectItem::Initialized);
        m_currentActions.append(m_showAnalDeviceAction);
        m_removeAnalDeviceAction->setDisabled(!m_analDevice->isManual() && m_analDevice->isAvailable());
        m_currentActions.append(m_removeAnalDeviceAction);

        m_currentDefaultAction = chooseDefaultAction(m_analDevice);
    }
    else if ((m_protocol = dynamic_cast<TProtocolModel *>(item))) {
        m_currentActions.append(m_editProtocolAction);
        m_currentDefaultAction = m_editProtocolAction;
    }
    else if ((m_scenario = dynamic_cast<TScenarioModel *>(item))) {
        m_currentActions.append(m_editScenarioAction);
        m_currentDefaultAction = m_editScenarioAction;
    }
    else if (dynamic_cast<TIODeviceContainer *>(item) && (m_component = dynamic_cast<TComponentModel *>(item->parent()))) {
        m_addIODeviceAction->setDisabled(m_component->status() != TProjectItem::Initialized || !m_component->canAddIODevice());
        m_currentActions.append(m_addIODeviceAction);

        m_currentDefaultAction = nullptr;
    }
    else if (dynamic_cast<TScopeContainer *>(item) && (m_component = dynamic_cast<TComponentModel *>(item->parent()))) {
        m_addScopeAction->setDisabled(m_component->status() != TProjectItem::Initialized || !m_component->canAddScope());
        m_currentActions.append(m_addScopeAction);

        m_currentDefaultAction = nullptr;
    }
    else if (dynamic_cast<TAnalDeviceContainer *>(item) && (m_component = dynamic_cast<TComponentModel *>(item->parent()))) {
        m_addAnalDeviceAction->setDisabled(m_component->status() != TProjectItem::Initialized || !m_component->canAddAnalDevice());
        m_currentActions.append(m_addAnalDeviceAction);

        m_currentDefaultAction = nullptr;
    }
    else if ((dynamic_cast<TProtocolContainer *>(item))) {
        m_openProtocolManagerAction->setEnabled(true);
        m_currentActions.append(m_openProtocolManagerAction);

        m_currentDefaultAction = m_openProtocolManagerAction;
    }
    else if ((dynamic_cast<TScenarioContainer *>(item))) {
        m_openScenarioManagerAction->setEnabled(true);
        m_currentActions.append(m_openScenarioManagerAction);

        m_currentDefaultAction = m_openScenarioManagerAction;
    }
    else if ((dynamic_cast<TComponentContainer *>(item))){
        m_currentActions.append(m_openDeviceAction);

        m_currentDefaultAction = m_openDeviceAction;
    }
    else {
        m_currentActions.append(m_openDeviceAction);
        m_currentActions.append(m_openProtocolManagerAction);
        m_currentActions.append(m_openScenarioManagerAction);
    }

    if (m_unit) {
        m_showInfoAction->setDisabled(m_unit->status() != TProjectItem::Initialized || m_unit->preInitParams().isEmpty());
        m_currentActions.append(m_showInfoAction);
    }

    if (!m_toolbar)
        return;

    m_toolbar->clear();
    m_toolbar->addActions(m_currentActions);
}

void TProjectView::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    QTreeView::selectionChanged(selected, deselected);

    TProjectItem * item = nullptr;

    if (!selected.indexes().isEmpty())
        item = static_cast<TProjectItem *>(selected.indexes().first().internalPointer());

    refreshActionsForItem(item);
}

void TProjectView::showContextMenu(const QPoint &point)
{
    QModelIndex index = indexAt(point);

    refreshActionsForItem(static_cast<TProjectItem *>(index.internalPointer()));

    QMenu * contextMenu = new QMenu(this);

    contextMenu->addActions(m_currentActions);

    if (contextMenu->isEmpty()) {
        delete contextMenu;
        return;
    }

    if (m_currentDefaultAction)
        contextMenu->setDefaultAction(m_currentDefaultAction);

    contextMenu->setAttribute(Qt::WA_DeleteOnClose, true);
    contextMenu->popup(viewport()->mapToGlobal(point));

    connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QObject::deleteLater);
}

void TProjectView::runDefaultAction(const QModelIndex & index) {
    if (!index.internalPointer()) {
        return;
    }

    TProjectItem * item = static_cast<TProjectItem *>(index.internalPointer());

    refreshActionsForItem(item);

    if (m_currentDefaultAction) {
        m_currentDefaultAction->trigger();
    }
}

QAction * TProjectView::chooseDefaultAction(TComponentModel * component)
{
    if (!component->isInit()) {
        return m_initComponentAction;
    }
    else {
        return m_showComponentSettingsAction;
    }
}

QAction * TProjectView::chooseDefaultAction(TIODeviceModel * IODevice)
{
    if (!IODevice->isInit()) {
        return m_initIODeviceAction;
    }
    else {
        return m_showIODeviceAction;
    }
}

QAction * TProjectView::chooseDefaultAction(TScopeModel * scope)
{
    if (!scope->isInit()) {
        return m_initScopeAction;
    }
    else {
        return m_showScopeAction;
    }
}

QAction * TProjectView::chooseDefaultAction(TAnalDeviceModel * analDevice)
{
    if (!analDevice->isInit()) {
        return m_initAnalDeviceAction;
    }
    else {
        return m_showAnalDeviceAction;
    }
}

void TProjectView::initComponent()
{
    if (!m_component || m_component->isInit()) {
        return;
    }

    TConfigParam param = m_component->preInitParams();

    if (!param.isEmpty()) {
        TInitComponentDialog dialog(m_component, this);

        if (dialog.exec() != QDialog::DialogCode::Accepted) {
            return;
        }
    }

    if (!m_component->init()) {
        TDialog::componentInitFailedGeneralMessage(this);
    }
}

void TProjectView::deinitComponent()
{
    if (!m_component || !m_component->isInit()) {
        return;
    }

    if (!TDialog::componentDeinitQuestion(this)) {
        return;
    }

    if (!m_component->deInit()) {
        TDialog::componentDeinitFailedGeneralMessage(this);
    }
}

void TProjectView::showComponentSettings()
{
    if (!m_component->isInit())
        return;

    TConfigParam param = m_component->postInitParams();
    if (!param.isEmpty()) {
        TComponentSettingsDialog dialog(m_component, this);

        if (dialog.exec() != QDialog::DialogCode::Accepted) {
            return;
        }
    }
}

void TProjectView::openDevice()
{
    TDeviceWizard * deviceWizard = new TDeviceWizard(qobject_cast<TProjectModel *>(model())->componentContainer(), this);
    deviceWizard->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    deviceWizard->show();
}

void TProjectView::addIODevice()
{
    if (!m_component || !m_component->isInit()) {
        return;
    }

    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info)) {
        return;
    }

    bool ok = m_component->addIODevice(name, info);
    if (!ok) {
        TDialog::deviceAddFailedGeneralMessage(this);
    }
}

void TProjectView::addScope()
{
    if (!m_component || !m_component->isInit()) {
        return;
    }

    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info)) {
        return;
    }

    bool ok = m_component->addScope(name, info);
    if (!ok) {
        TDialog::deviceAddFailedGeneralMessage(this);
    }
}

void TProjectView::addAnalDevice()
{
    if (!m_component || !m_component->isInit()) {
        return;
    }

    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info)) {
        return;
    }

    bool ok = m_component->addAnalDevice(name, info);
    if (!ok) {
        TDialog::deviceAddFailedGeneralMessage(this);
    }
}

void TProjectView::initIODevice()
{
    if (!m_IODevice || m_IODevice->isInit()) {
        return;
    }

    TConfigParam param = m_IODevice->preInitParams();

    if (!param.isEmpty()) {
        TInitIODeviceDialog dialog(m_IODevice, this);

        if (dialog.exec() != QDialog::DialogCode::Accepted) {
            return;
        }
    }

    if (!m_IODevice->init()) {
        TDialog::deviceInitFailedGeneralMessage(this);
    }
    else {
        m_IODevice->show();
    }
}

void TProjectView::deinitIODevice()
{
    if (!m_IODevice|| !m_IODevice->isInit()) {
        return;
    }

    if (!m_IODevice->deInit()) {
        TDialog::deviceDeinitFailedGeneralMessage(this);
    }
}

void TProjectView::showIODevice()
{
    m_IODevice->show();
}

void TProjectView::removeIODevice()
{
    m_IODevice->remove();
}

void TProjectView::initScope()
{
    if (!m_scope || m_scope->isInit()) {
        return;
    }

    TConfigParam param = m_scope->preInitParams();

    if (!param.isEmpty()) {
        TInitScopeDialog dialog(m_scope, this);

        if (dialog.exec() != QDialog::DialogCode::Accepted) {
            return;
        }
    }

    if (!m_scope->init()) {
        TDialog::deviceInitFailedGeneralMessage(this);
    }
    else {
        m_scope->show();
    }
}

void TProjectView::deinitScope()
{
    if (!m_scope|| !m_scope->isInit()) {
        return;
    }

    if (!m_scope->deInit()) {
        TDialog::deviceDeinitFailedGeneralMessage(this);
    }
}

void TProjectView::showScope()
{
    m_scope->show();
}

void TProjectView::removeScope()
{
    m_scope->remove();
}

void TProjectView::initAnalDevice()
{
    if (!m_analDevice || m_analDevice->isInit()) {
        return;
    }

    TConfigParam param = m_analDevice->preInitParams();

    if (!param.isEmpty()) {
        TInitAnalDeviceDialog dialog(m_analDevice, this);

        if (dialog.exec() != QDialog::DialogCode::Accepted) {
            return;
        }
    }

    if (!m_analDevice->init()) {
        TDialog::deviceInitFailedGeneralMessage(this);
    }
    else {
        m_analDevice->show();
    }
}

void TProjectView::deinitAnalDevice()
{
    if (!m_analDevice|| !m_analDevice->isInit()) {
        return;
    }

    if (!m_analDevice->deInit()) {
        TDialog::deviceDeinitFailedGeneralMessage(this);
    }
}

void TProjectView::showAnalDevice()
{
    m_analDevice->show();
}

void TProjectView::removeAnalDevice()
{
    m_analDevice->remove();
}

void TProjectView::showInfo()
{
    if (!m_unit || !m_unit->isInit() || m_unit->preInitParams().isEmpty())
        return;

    TPluginUnitInfoDialog dialog(m_unit, this);

    dialog.exec();
}

void TProjectView::showProtocolManager()
{
    ((TProjectModel *)model())->protocolContainer()->showManager();
}

void TProjectView::showScenarioManager()
{
    ((TProjectModel *)model())->scenarioContainer()->showManager();
}

void TProjectView::openProtocolEditor()
{
    if (!m_protocol) {
        return;
    }

    m_protocol->openEditor();
}

void TProjectView::openScenarioEditor()
{
    if (!m_scenario) {
        return;
    }

    m_scenario->openEditor();
}

