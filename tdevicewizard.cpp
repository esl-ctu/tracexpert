#include "tdevicewizard.h"

#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QCheckBox>

#include "tdialog.h"

TDeviceWizard::TDeviceWizard(TComponentContainer * componentContainer, QWidget * parent, TComponentModel * component, Qt::WindowFlags flags)
    : QWizard(parent, flags), m_componentContainer(componentContainer), m_selectedComponent(component)
{
    setWindowTitle(tr("Open Device Wizard"));

    setMinimumSize(600, 650);

    setWindowModality(Qt::WindowModal);

    setOptions(QWizard::NoCancelButtonOnLastPage);

    if (!m_selectedComponent) {
        setPage(Page_SelectComponent, new TSelectComponentWizardPage(this));
        setPage(Page_InitComponent, new TInitComponentWizardPage(this));
        setPage(Page_PostInitComponent, new TPostInitComponentWizardPage(this));
    }
    setPage(Page_SelectDevice, new TSelectDeviceWizardPage(this));
    setPage(Page_InitDevice, new TInitDeviceWizardPage(this));
    setPage(Page_Final, new TFinalWizardPage(this));
}

TDeviceWizardPage::TDeviceWizardPage(TDeviceWizard * parent)
    : QWizardPage(parent), m_componentContainer(parent->m_componentContainer), m_selectedComponent(parent->m_selectedComponent), m_selectedDevice(parent->m_selectedDevice)
{

}

bool TDeviceWizardPage::initComponent(TConfigParam * param)
{
    if (m_selectedComponent->isInit()) {
        if (TDialog::componentReinitQuestion(this)) {
            bool ok = deInitComponent();
            if (!ok) {
                return false;
            }
        }
        else {
            return true;
        }
    }

    if (param) {
        *param = m_selectedComponent->setPreInitParams(*param);

        if (!checkParam(*param)) {
            return false;
        }
    }

    bool ok = m_selectedComponent->init();
    if (!ok) {
        TDialog::componentInitFailedGeneralMessage(this);
    }

    return ok;
}

bool TDeviceWizardPage::deInitComponent()
{
    if (!m_selectedComponent->isInit()) {
        return true;
    }

    bool ok = m_selectedComponent->deInit();
    if (!ok) {
        TDialog::componentDeinitFailedGeneralMessage(this);
    }

    return ok;
}

bool TDeviceWizardPage::initDevice(TConfigParam * param)
{
    if (m_selectedDevice->isInit()) {
        if (TDialog::deviceReinitQuestion(this)) {
            bool ok = deInitDevice();
            if (!ok) {
                return false;
            }
        }
        else {
            return false;
        }
    }

    if (param) {
        *param = m_selectedDevice->setPreInitParams(*param);

        if (!checkParam(*param)) {
            return false;
        }
    }

    bool ok;
    if (m_selectedDevice) {
        ok = m_selectedDevice->init();
    }
    else {
        TDialog::deviceInitFailedNoSelectionMessage(this);
        return false;
    }

    if (!ok) {
        TDialog::deviceInitFailedGeneralMessage(this);
    }
    else {
        m_selectedDevice->show();
    }

    return ok;
}

bool TDeviceWizardPage::deInitDevice()
{
    if (!m_selectedDevice->isInit()) {
        return true;
    }

    bool ok;
    if (m_selectedDevice) {
        ok = m_selectedDevice->deInit();
    }
    else {
        TDialog::deviceDeinitFailedNoSelectionMessage(this);
        return false;
    }

    if (!ok) {
        TDialog::deviceDeinitFailedGeneralMessage(this);
    }

    return ok;
}

bool TDeviceWizardPage::checkParam(const TConfigParam & param)
{
    if (param.getState(true) == TConfigParam::TState::TError) {
        return false;
    }
    else if (param.getState(true) == TConfigParam::TState::TWarning) {
        if (!TDialog::paramWarningQuestion(this)) {
            return false;
        }
    }
    return true;
}

TSelectComponentWizardPage::TSelectComponentWizardPage(TDeviceWizard * parent)
    : TDeviceWizardPage(parent)
{
    m_componentListWidget = new TPluginUnitContainerView(this);

    m_reInitCheckBox = new QCheckBox(tr("Reinitialize"));
    m_reInitCheckBox->setChecked(false);
    m_reInitCheckBox->setEnabled(false);

    QLayout * selectComponentLayout = new QVBoxLayout;
    selectComponentLayout->addWidget(m_componentListWidget);
    selectComponentLayout->addWidget(m_reInitCheckBox);

    setLayout(selectComponentLayout);
}

void TSelectComponentWizardPage::initializePage()
{
    setTitle(tr("Select Component"));
    setSubTitle(tr("Select the component you want to use for opening the device"));

    if (m_componentListWidget->selectionModel()) {
        disconnect(m_componentListWidget->selectionModel(), nullptr, this, nullptr);
    }
    disconnect(m_componentListWidget, nullptr, wizard(), nullptr);

    m_componentListWidget->setModel(m_componentContainer);

    connect(m_componentListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TSelectComponentWizardPage::selectComponent);
    connect(m_componentListWidget, &QTableView::doubleClicked, wizard(), &QWizard::next);
}

bool TSelectComponentWizardPage::isComplete() const
{
    return m_componentListWidget->selectionModel()->selectedRows().length() > 0;
}

int TSelectComponentWizardPage::nextId() const
{
    if (m_selectedComponent) {
        TConfigParam componentPreInitParam = m_selectedComponent->preInitParams();
        if (!m_selectedComponent->isInit() && !componentPreInitParam.isEmpty()) {
            return TDeviceWizard::Page_InitComponent;
        }
        
        TConfigParam componentPostInitParam = m_selectedComponent->postInitParams();
        if (!componentPostInitParam.isEmpty()) {
            return TDeviceWizard::Page_PostInitComponent;
        }
    }
    return TDeviceWizard::Page_SelectDevice;
}

bool TSelectComponentWizardPage::validatePage()
{
    if (!m_selectedComponent) {
        return false;
    }

    if (m_reInitCheckBox->isChecked()) {
        deInitComponent();
    }

    if (!m_selectedComponent->isInit() && nextId() != TDeviceWizard::Page_InitComponent) {
        return initComponent();
    }

    return true;
}

void TSelectComponentWizardPage::selectComponent()
{
    QModelIndexList selected = m_componentListWidget->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        m_selectedComponent = nullptr;
    }
    else {
        m_selectedComponent = m_componentContainer->at(selected.constFirst().row());
    }

    m_reInitCheckBox->setEnabled(m_selectedComponent);

    emit completeChanged();
}

TInitComponentWizardPage::TInitComponentWizardPage(TDeviceWizard * parent)
    : TDeviceWizardPage(parent)
{
    m_componentPreInitParamWidget = new TConfigParamWidget(TConfigParam());

    QLayout * initComponentLayout = new QVBoxLayout;
    initComponentLayout->addWidget(m_componentPreInitParamWidget);

    setLayout(initComponentLayout);
}

void TInitComponentWizardPage::initializePage()
{
    setTitle(tr("%1 Initialization").arg(m_selectedComponent->name()));
    setSubTitle(tr("%1 component needs to be initialized before usage").arg(m_selectedComponent->name()));
    
    TConfigParam componentPreInitParam = m_selectedComponent->preInitParams();

    m_componentPreInitParamWidget->setParam(componentPreInitParam);
}

bool TInitComponentWizardPage::isComplete() const
{
    return true;
}

int TInitComponentWizardPage::nextId() const
{
    TConfigParam componentPostInitParam = m_selectedComponent->postInitParams();
    if (!componentPostInitParam.isEmpty()) {
        return TDeviceWizard::Page_PostInitComponent;
    }
    return TDeviceWizard::Page_SelectDevice;
}

bool TInitComponentWizardPage::validatePage()
{
    bool ok;

    TConfigParam preInitParam = m_componentPreInitParamWidget->param(&ok);
    if (!ok) {
        return false;
    }

    ok = initComponent(&preInitParam);

    m_componentPreInitParamWidget->setParam(preInitParam);

    return ok;
}

TPostInitComponentWizardPage::TPostInitComponentWizardPage(TDeviceWizard * parent)
    : TDeviceWizardPage(parent)
{
    m_componentPostInitParamWidget = new TConfigParamWidget(TConfigParam());

    QLayout * postInitComponentLayout = new QVBoxLayout;
    postInitComponentLayout->addWidget(m_componentPostInitParamWidget);

    setLayout(postInitComponentLayout);
}

void TPostInitComponentWizardPage::initializePage()
{
    setTitle(tr("%1 Settings").arg(m_selectedComponent->name()));
    setSubTitle(tr("%1 component has been initialized. Some more settings are needed before usage").arg(m_selectedComponent->name()));
    
    TConfigParam componentPostInitParam = m_selectedComponent->postInitParams();

    m_componentPostInitParamWidget->setParam(componentPostInitParam);
}

bool TPostInitComponentWizardPage::isComplete() const
{
    return true;
}

int TPostInitComponentWizardPage::nextId() const
{
    return TDeviceWizard::Page_SelectDevice;
}

bool TPostInitComponentWizardPage::validatePage()
{
    bool ok;
    TConfigParam postInitParam = m_componentPostInitParamWidget->param(&ok);
    if (!ok) {
        return false;
    }

    postInitParam = m_selectedComponent->setPostInitParams(postInitParam);

    return checkParam(postInitParam);
}

TSelectDeviceWizardPage::TSelectDeviceWizardPage(TDeviceWizard * parent)
    : TDeviceWizardPage(parent)
{
    m_IODeviceListWidget = new TPluginUnitContainerView(this);

    m_scopeListWidget = new TPluginUnitContainerView(this);

    m_analDeviceListWidget = new TPluginUnitContainerView(this);

    m_addIOButton = new QPushButton("Add");
    connect(m_addIOButton, &QPushButton::clicked, this, &TSelectDeviceWizardPage::addIODevice);

    QLayout * IOLayout = new QVBoxLayout;
    IOLayout->addWidget(m_IODeviceListWidget);
    IOLayout->addWidget(m_addIOButton);

    m_IOGroupBox = new QGroupBox("IO devices");
    m_IOGroupBox->setLayout(IOLayout);

    m_addScopeButton = new QPushButton("Add");
    connect(m_addScopeButton, &QPushButton::clicked, this, &TSelectDeviceWizardPage::addScope);

    QLayout * scopeLayout = new QVBoxLayout;
    scopeLayout->addWidget(m_scopeListWidget);
    scopeLayout->addWidget(m_addScopeButton);

    m_scopeGroupBox = new QGroupBox("Oscilloscopes");
    m_scopeGroupBox->setLayout(scopeLayout);

    m_addAnalButton = new QPushButton("Add");
    connect(m_addAnalButton, &QPushButton::clicked, this, &TSelectDeviceWizardPage::addAnalDevice);

    QLayout * analLayout = new QVBoxLayout;
    analLayout->addWidget(m_analDeviceListWidget);
    analLayout->addWidget(m_addAnalButton);

    m_analGroupBox = new QGroupBox("Analytical devices");
    m_analGroupBox->setLayout(analLayout);

    QLayout * selectDeviceLayout = new QHBoxLayout;
    selectDeviceLayout->addWidget(m_IOGroupBox);
    selectDeviceLayout->addWidget(m_scopeGroupBox);
    selectDeviceLayout->addWidget(m_analGroupBox);

    setLayout(selectDeviceLayout);
}

void TSelectDeviceWizardPage::initializePage()
{
    setTitle(tr("Select Device"));

    if (m_IODeviceListWidget->selectionModel()) {
        disconnect(m_IODeviceListWidget->selectionModel(), nullptr, this, nullptr);
    }

    if (m_scopeListWidget->selectionModel()) {
        disconnect(m_scopeListWidget->selectionModel(), nullptr, this, nullptr);
    }

    if (m_analDeviceListWidget->selectionModel()) {
        disconnect(m_analDeviceListWidget->selectionModel(), nullptr, this, nullptr);
    }

    disconnect(m_IODeviceListWidget, nullptr, wizard(), nullptr);
    disconnect(m_scopeListWidget, nullptr, wizard(), nullptr);
    disconnect(m_analDeviceListWidget, nullptr, wizard(), nullptr);
    
    TIODeviceContainer * IODevices = m_selectedComponent->IODeviceContainer();
    TScopeContainer * scopes = m_selectedComponent->scopeContainer();
    TAnalDeviceContainer * analDevices = m_selectedComponent->analDeviceContainer();

    m_IODeviceListWidget->setModel(IODevices);
    m_scopeListWidget->setModel(scopes);
    m_analDeviceListWidget->setModel(analDevices);

    connect(m_IODeviceListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TSelectDeviceWizardPage::selectIODevice);
    connect(m_scopeListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TSelectDeviceWizardPage::selectScope);
    connect(m_analDeviceListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &TSelectDeviceWizardPage::selectAnalDevice);
    connect(m_IODeviceListWidget, &QTableView::doubleClicked, wizard(), &QWizard::next);
    connect(m_scopeListWidget, &QTableView::doubleClicked, wizard(), &QWizard::next);
    connect(m_analDeviceListWidget, &QTableView::doubleClicked, wizard(), &QWizard::next);
    
    if (IODevices->count() || m_selectedComponent->canAddIODevice()) {
        m_IOGroupBox->show();
    }
    else {
        m_IOGroupBox->hide();
    }

    m_addIOButton->setEnabled(m_selectedComponent->canAddIODevice());
    
    if (scopes->count() ||  m_selectedComponent->canAddScope()) {
        m_scopeGroupBox->show();
    }
    else {
        m_scopeGroupBox->hide();
    }

    m_addScopeButton->setEnabled(m_selectedComponent->canAddScope());

    if (analDevices->count() || m_selectedComponent->canAddAnalDevice()) {
        m_analGroupBox->show();
    }
    else {
        m_analGroupBox->hide();
    }

    m_addAnalButton->setEnabled(m_selectedComponent->canAddAnalDevice());

    if (m_IOGroupBox->isHidden() && m_scopeGroupBox->isHidden() && m_analGroupBox->isHidden()) {
        setSubTitle(tr("No device can be opened using selected component!"));
    }
    else {
        setSubTitle(tr("Select the device you want to open"));
    }
}

bool TSelectDeviceWizardPage::isComplete() const
{
    return 1 ==
           m_IODeviceListWidget->selectionModel()->selectedRows().length() +
           m_scopeListWidget->selectionModel()->selectedRows().length() +
           m_analDeviceListWidget->selectionModel()->selectedRows().length();
}

int TSelectDeviceWizardPage::nextId() const
{
    if (m_selectedDevice) {
        TConfigParam devicePreInitParam = m_selectedDevice->preInitParams();

        if (!devicePreInitParam.isEmpty()) {
            return TDeviceWizard::Page_InitDevice;
        }
    }

    return TDeviceWizard::Page_Final;
}

bool TSelectDeviceWizardPage::validatePage()
{
    if (!m_selectedDevice) {
        TDialog::deviceInitFailedNoSelectionMessage(this);
        return false;
    }

    if (!m_selectedDevice->isInit() && nextId() != TDeviceWizard::Page_InitDevice) {
        return initDevice();
    }

    return true;
}

void TSelectDeviceWizardPage::selectIODevice()
{
    m_scopeListWidget->selectionModel()->clearSelection();
    m_analDeviceListWidget->selectionModel()->clearSelection();

    QModelIndexList selected = m_IODeviceListWidget->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        m_selectedDevice = nullptr;
    }
    else {
        m_selectedDevice = m_selectedComponent->IODeviceContainer()->at(selected.constFirst().row());
    }

    emit completeChanged();
}

void TSelectDeviceWizardPage::selectScope()
{
    m_IODeviceListWidget->selectionModel()->clearSelection();
    m_analDeviceListWidget->selectionModel()->clearSelection();

    QModelIndexList selected = m_scopeListWidget->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        m_selectedDevice = nullptr;
    }
    else {
        m_selectedDevice = m_selectedComponent->scopeContainer()->at(selected.constFirst().row());
    }

    emit completeChanged();
}

void TSelectDeviceWizardPage::selectAnalDevice()
{
    m_IODeviceListWidget->selectionModel()->clearSelection();
    m_scopeListWidget->selectionModel()->clearSelection();

    QModelIndexList selected = m_analDeviceListWidget->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        m_selectedDevice = nullptr;
    }
    else {
        m_selectedDevice = m_selectedComponent->analDeviceContainer()->at(selected.constFirst().row());
    }

    emit completeChanged();
}

bool TSelectDeviceWizardPage::addIODevice()
{
    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info)) {
        return false;
    }

    bool ok = m_selectedComponent->addIODevice(name, info);
    if (!ok) {
        TDialog::deviceAddFailedGeneralMessage(this);
    }

    return ok;
}

bool TSelectDeviceWizardPage::addScope()
{
    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info)) {
        return false;
    }

    bool ok = m_selectedComponent->addScope(name, info);
    if (!ok) {
        TDialog::deviceAddFailedGeneralMessage(this);
    }

    return ok;
}

bool TSelectDeviceWizardPage::addAnalDevice()
{
    QString name;
    QString info;

    if (!TDialog::addDeviceDialog(this, name, info)) {
        return false;
    }

    bool ok = m_selectedComponent->addAnalDevice(name, info);
    if (!ok) {
        TDialog::deviceAddFailedGeneralMessage(this);
    }

    return ok;
}

TInitDeviceWizardPage::TInitDeviceWizardPage(TDeviceWizard * parent)
    : TDeviceWizardPage(parent)
{
    m_devicePreInitParamWidget = new TConfigParamWidget(TConfigParam());

    QLayout * initDeviceLayout = new QVBoxLayout;
    initDeviceLayout->addWidget(m_devicePreInitParamWidget);

    setLayout(initDeviceLayout);
}

void TInitDeviceWizardPage::initializePage()
{
    setTitle(tr("%1 Initialization").arg(m_selectedDevice->name()));
    setSubTitle(tr("%1 device needs to be initialized before usage").arg(m_selectedDevice->name()));
    
    TConfigParam devicePreInitParam = m_selectedDevice->preInitParams();

    m_devicePreInitParamWidget->setParam(devicePreInitParam);
}

bool TInitDeviceWizardPage::isComplete() const
{
    return true;
}

int TInitDeviceWizardPage::nextId() const
{
    return TDeviceWizard::Page_Final;
}

bool TInitDeviceWizardPage::validatePage()
{
    bool ok;

    TConfigParam preInitParam = m_devicePreInitParamWidget->param(&ok);
    if (!ok) {
        return false;
    }

    ok = initDevice(&preInitParam);

    m_devicePreInitParamWidget->setParam(preInitParam);

    return ok;
}

TFinalWizardPage::TFinalWizardPage(TDeviceWizard * parent) :
    TDeviceWizardPage(parent)
{
    setFinalPage(true);
}

void TFinalWizardPage::initializePage()
{
    setTitle(tr("%1 Success").arg(m_selectedDevice->name()));
    setSubTitle(tr("%1 device has been successfuly initialized").arg(m_selectedDevice->name()));
}

bool TFinalWizardPage::isComplete() const
{
    return true;
}

int TFinalWizardPage::nextId() const
{
    return -1;
}

bool TFinalWizardPage::validatePage()
{
    return true;
}
