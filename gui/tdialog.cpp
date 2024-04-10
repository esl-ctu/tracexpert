#include "tdialog.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>

bool TDialog::addDeviceDialog(QWidget * parent, QString &name, QString &info)
{
    QDialog * addDeviceDialog = new QDialog(parent);
    QLabel * nameLabel = new QLabel(parent->tr("Name"));
    QLabel * infoLabel = new QLabel(parent->tr("Info"));

    QLineEdit * nameEdit = new QLineEdit;
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

void TDialog::deviceFailedBusyMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Device busy"), parent->tr("Operation failed because device is currently busy!"));
}

void TDialog::deviceReceiveFailedMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Receive failed"), parent->tr("Operation failed because device is unable to read data!"));
}

void TDialog::deviceSendFailedMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Send failed"), parent->tr("Operation failed because device is unable to write data!"));
}

void TDialog::parameterValueEmpty(QWidget * parent, const QString & parameterName)
{
    criticalMessage(parent, parent->tr("Parameter value is empty"), parent->tr("Parameter \"%1\" cannot be empty!").arg(parameterName));
}

void TDialog::parameterValueInvalid(QWidget * parent, const QString & parameterName)
{
    criticalMessage(parent, parent->tr("Parameter value is invalid"), parent->tr("Parameter \"%1\" has an invalid value!").arg(parameterName));
}

void TDialog::parameterValueNotUniqueMessage(QWidget * parent, const QString & parameterName)
{
    criticalMessage(parent, parent->tr("Parameter value is non-unique"), parent->tr("The value of \"%1\" has to be unique!").arg(parameterName));
}

void TDialog::protocolMessageCouldNotBeFormed(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Send failed"), parent->tr("Protocol message could not be formed, check console for errors!"));
}

bool TDialog::question(QWidget * parent, const QString &title, const QString &text)
{
    return QMessageBox::question(parent, title, text) == QMessageBox::Yes;
}

void TDialog::criticalMessage(QWidget * parent, const QString &title, const QString &text)
{
    QMessageBox::critical(parent, title, text);
}

TConfigParamDialog::TConfigParamDialog(QString acceptText, QString title, TPluginUnitModel * unit, bool preInit, QWidget * parent)
    : QDialog(parent), m_unit(unit), m_preInit(preInit)
{
    TConfigParam param = m_preInit ? m_unit->preInitParams() : m_unit->postInitParams();

    m_paramWidget = new TConfigParamWidget(param);
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
    TConfigParam param = m_preInit ? m_unit->setPreInitParams(m_paramWidget->param()) : m_unit->setPostInitParams(m_paramWidget->param());

    TConfigParam::TState state = param.getState();
    if (state == TConfigParam::TState::TError) {
        m_paramWidget->setParam(param);
        return;
    }
    else if (state == TConfigParam::TState::TWarning) {
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
    : TConfigParamDialog(tr("Initialize"), tr("Initialize Component"), component, true, parent)
{
}

TComponentSettingsDialog::TComponentSettingsDialog(TComponentModel * component, QWidget * parent)
    : TConfigParamDialog(tr("Apply"), tr("Component Settings"), component, false, parent)
{
}

TInitIODeviceDialog::TInitIODeviceDialog(TIODeviceModel * IOdevice, QWidget * parent)
    : TConfigParamDialog(tr("Initialize"), tr("Initialize IO Device"), IOdevice, true, parent)
{
}

TInitScopeDialog::TInitScopeDialog(TScopeModel * scope, QWidget * parent)
    : TConfigParamDialog(tr("Initialize"), tr("Initialize Oscilloscope"), scope, true, parent)
{
}

TPluginUnitInfoDialog::TPluginUnitInfoDialog(TPluginUnitModel * unit, QWidget * parent)
{
    setWindowTitle(unit->name());

    QLabel * label = new QLabel(unit->info(), this);

    TConfigParamWidget * paramWidget = new TConfigParamWidget(unit->preInitParams(), parent, true);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);

    parent->connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);

    QVBoxLayout * dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(label);
    dialogLayout->addWidget(paramWidget);
    dialogLayout->addWidget(buttonBox);

    setLayout(dialogLayout);
}
