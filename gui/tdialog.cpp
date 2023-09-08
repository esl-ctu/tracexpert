#include "tdialog.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

#include "tconfigparamwidget.h"

bool TDialog::addDeviceDialog(QWidget * parent, QString &name, QString &info)
{
    QDialog * addDeviceDialog = new QDialog(parent);
    QLabel * nameLabel = new QLabel(parent->tr("Name"));
    QLabel * infoLabel = new QLabel(parent->tr("Info"));

    QLineEdit * nameEdit = new QLineEdit;
    //nameEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(".+")));
    QLineEdit * infoEdit = new QLineEdit;

    QGridLayout * dialogGridLayout = new QGridLayout;
    dialogGridLayout->addWidget(nameLabel, 0, 0);
    dialogGridLayout->addWidget(nameEdit, 0, 1);
    dialogGridLayout->addWidget(infoLabel, 1, 0);
    dialogGridLayout->addWidget(infoEdit, 1, 1);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Add");
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    parent->connect(nameEdit, &QLineEdit::textChanged, parent, [=]() {buttonBox->button(QDialogButtonBox::Ok)->setEnabled(nameEdit->hasAcceptableInput());});

    parent->connect(buttonBox, &QDialogButtonBox::accepted, addDeviceDialog, &QDialog::accept);
    parent->connect(buttonBox, &QDialogButtonBox::rejected, addDeviceDialog, &QDialog::reject);

    QVBoxLayout * dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(dialogGridLayout);
    dialogLayout->addWidget(buttonBox);

    addDeviceDialog->setLayout(dialogLayout);

    addDeviceDialog->adjustSize();
    addDeviceDialog->exec();

    name = nameEdit->text();
    info = infoEdit->text();

    return (addDeviceDialog->result() == QDialog::Accepted && nameEdit->text() != QString(""));
}

bool TDialog::paramWarningQuestion(QWidget * parent)
{
    return question(parent, parent->tr("Parameter Warning"), parent->tr("There are some paramater values causing warnings. Do you want to proceed anyway?"));
}

bool TDialog::componentReinitQuestion(QWidget * parent)
{
    return question(parent, parent->tr("Component Initialized"), parent->tr("The selected component is already initialized. Do you want to reinitialize it? Warning: Reinitialization of a component closes all opened devices managed by the component!"));
}

bool TDialog::componentDeinitQuestion(QWidget * parent)
{
    return question(parent, parent->tr("Component Initialized"), parent->tr("The selected component will be deinitialized. Deinitialization of a component closes all opened devices managed by the component. Do you want to continue?"));
}

bool TDialog::deviceReinitQuestion(QWidget * parent)
{
    return question(parent, parent->tr("Device Initialized"), parent->tr("The selected device is already initialized. Do you want to reinitialize it?"));
}

void TDialog::componentInitFailedGeneralMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Component init failed"), parent->tr("Unable to initialize selected component!"));
}

void TDialog::componentDeinitFailedGeneralMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Component deinit failed"), parent->tr("Unable to deinitialize selected component!"));
}

void TDialog::deviceInitFailedGeneralMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Device init failed"), parent->tr("Unable to initialize selected device!"));
}

void TDialog::deviceInitFailedNoSelectionMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Device init failed"), parent->tr("No IO Device nor Scope is selected!"));
}

void TDialog::deviceDeinitFailedGeneralMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Device deinit failed"), parent->tr("Unable to deinitialize selected device!"));
}

void TDialog::deviceDeinitFailedNoSelectionMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Device deinit failed"), parent->tr("No IO Device nor Scope is selected!"));
}

void TDialog::deviceAddFailedGeneralMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Add device failed"), parent->tr("Unable to add device!"));
}

void TDialog::deviceFailedBusyMessage(QWidget *parent)
{
    criticalMessage(parent, parent->tr("Device busy"), parent->tr("Operation failed because device is currently busy!"));
}

bool TDialog::question(QWidget * parent, const QString &title, const QString &text)
{
    return QMessageBox::question(parent, title, text) == QMessageBox::Yes;
}

void TDialog::criticalMessage(QWidget * parent, const QString &title, const QString &text)
{
    QMessageBox::critical(parent, title, text);
}

TConfigParamDialog::TConfigParamDialog(QString acceptText, QString title, QWidget * parent)
    : QDialog(parent)
{
    m_paramWidget = new TConfigParamWidget(TConfigParam());
    setWindowTitle(title);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText(acceptText);

    parent->connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    parent->connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout * dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(m_paramWidget);
    dialogLayout->addWidget(buttonBox);

    setLayout(dialogLayout);
}

void TConfigParamDialog::accept()
{
    TConfigParam param = setParam();

    TConfigParam::TState state = param.getState();
    if (state == TConfigParam::TState::TError) {
        m_paramWidget->setParam(param);
        return;
    }
    else if (state == TConfigParam::TState::TError) {
        if (!TDialog::paramWarningQuestion(this)) {
            m_paramWidget->setParam(param);
            return;
        }
    }
    else {
        QDialog::accept();
    }
}

TInitComponentDialog::TInitComponentDialog(TComponentModel * component, QWidget * parent)
    : TConfigParamDialog(tr("Initialize"), tr("Initialize Component"), parent), m_component(component)
{
    m_paramWidget->setParam(param());
    adjustSize();
}

TConfigParam TInitComponentDialog::param()
{
    return m_component->preInitParams();
}

TConfigParam TInitComponentDialog::setParam()
{
    return m_component->setPreInitParams(m_paramWidget->param());
}

TComponentSettingsDialog::TComponentSettingsDialog(TComponentModel * component, QWidget * parent)
    : TConfigParamDialog(tr("Apply"), tr("Component Settings"), parent), m_component(component)
{
    m_paramWidget->setParam(param());
}

TConfigParam TComponentSettingsDialog::param()
{
    return m_component->postInitParams();
}

TConfigParam TComponentSettingsDialog::setParam()
{
    return m_component->setPostInitParams(m_paramWidget->param());
}

TInitIODeviceDialog::TInitIODeviceDialog(TIODeviceModel * IOdevice, QWidget * parent)
    : TConfigParamDialog(tr("Initialize"), tr("Initialize IO Device"), parent), m_IODevice(IOdevice)
{
    m_paramWidget->setParam(param());
    adjustSize();
}

TConfigParam TInitIODeviceDialog::param()
{
    return m_IODevice->preInitParams();
}

TConfigParam TInitIODeviceDialog::setParam()
{
    return m_IODevice->setPreInitParams(m_paramWidget->param());
}

TInitScopeDialog::TInitScopeDialog(TScopeModel * scope, QWidget * parent)
    : TConfigParamDialog(tr("Initialize"), tr("Initialize Oscilloscope"), parent), m_scope(scope)
{
    m_paramWidget->setParam(param());
    adjustSize();
}

TConfigParam TInitScopeDialog::param()
{
    return m_scope->preInitParams();
}

TConfigParam TInitScopeDialog::setParam()
{
    return m_scope->setPreInitParams(m_paramWidget->param());
}
