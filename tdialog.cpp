#include "tdialog.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QMessageBox>
#include <QCoreApplication>

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

bool TDialog::renameDeviceDialog(QWidget * parent, QString &name, QString &info)
{
    QDialog * addDeviceDialog = new QDialog(parent);
    QLabel * nameLabel = new QLabel(parent->tr("Name"));
    QLabel * infoLabel = new QLabel(parent->tr("Info"));

    QLineEdit * nameEdit = new QLineEdit(name);
    QLineEdit * infoEdit = new QLineEdit(info);

    QGridLayout * dialogGridLayout = new QGridLayout;
    dialogGridLayout->addWidget(nameLabel, 0, 0);
    dialogGridLayout->addWidget(nameEdit, 0, 1);
    dialogGridLayout->addWidget(infoLabel, 1, 0);
    dialogGridLayout->addWidget(infoEdit, 1, 1);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Rename");
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    parent->connect(nameEdit, &QLineEdit::textChanged, parent, [=]() {buttonBox->button(QDialogButtonBox::Ok)->setEnabled(nameEdit->hasAcceptableInput());});
    parent->connect(infoEdit, &QLineEdit::textChanged, parent, [=]() {buttonBox->button(QDialogButtonBox::Ok)->setEnabled(infoEdit->hasAcceptableInput());});

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

bool TDialog::exportImageDimensionsDialog(QWidget * parent, uint &width, uint &height)
{
    QDialog * addDeviceDialog = new QDialog(parent);
    QLabel * widthLabel = new QLabel(parent->tr("Width"));
    QLabel * heightLabel = new QLabel(parent->tr("Height"));

    QLineEdit * widthEdit = new QLineEdit(QString::number(width));
    QLineEdit * heightEdit = new QLineEdit(QString::number(height));

    QGridLayout * dialogGridLayout = new QGridLayout;
    dialogGridLayout->addWidget(widthLabel, 0, 0);
    dialogGridLayout->addWidget(widthEdit, 0, 1);
    dialogGridLayout->addWidget(heightLabel, 1, 0);
    dialogGridLayout->addWidget(heightEdit, 1, 1);

    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    buttonBox->button(QDialogButtonBox::Ok)->setText("Continue");

    parent->connect(widthEdit, &QLineEdit::textChanged, parent, [=]() {
        bool ok;
        widthEdit->text().toUInt(&ok);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
    });

    parent->connect(heightEdit, &QLineEdit::textChanged, parent, [=]() {
        bool ok;
        heightEdit->text().toUInt(&ok);
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(ok);
    });

    parent->connect(buttonBox, &QDialogButtonBox::accepted, addDeviceDialog, &QDialog::accept);
    parent->connect(buttonBox, &QDialogButtonBox::rejected, addDeviceDialog, &QDialog::reject);

    QVBoxLayout * dialogLayout = new QVBoxLayout;
    dialogLayout->addLayout(dialogGridLayout);
    dialogLayout->addWidget(buttonBox);

    addDeviceDialog->setLayout(dialogLayout);

    addDeviceDialog->adjustSize();
    addDeviceDialog->exec();

    width = widthEdit->text().toUInt();
    height = heightEdit->text().toUInt();

    return (addDeviceDialog->result() == QDialog::Accepted);
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

void TDialog::paramValueErrorMessage(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Parameter Error"), parent->tr("There are some paramater values causing errors!"));
}

void TDialog::protocolMessageCouldNotBeFormed(QWidget * parent)
{
    criticalMessage(parent, parent->tr("Send failed"), parent->tr("Protocol message could not be formed, check console for errors!"));
}

bool TDialog::closeConfirmation(QWidget * parent, QString closedObjectName)
{
    if (closedObjectName.isEmpty())
        closedObjectName = "this window";

    return QMessageBox::question(
        parent,
        parent->tr("Close confirmation"),
        parent->tr("Are you sure you want to close %1? All unsaved changes will be lost.").arg(closedObjectName),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No) == QMessageBox::Yes;
}

bool TDialog::scenarioTerminationConfirmation(QWidget * parent)
{
    return QMessageBox::question(
               parent,
               parent->tr("Terminate"),
               parent->tr("Are you sure you want to terminate scenario execution?"),
               QMessageBox::Yes | QMessageBox::No,
               QMessageBox::No) == QMessageBox::Yes;
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

    if (state == TConfigParam::TState::TWarning) {
        if (!TDialog::paramWarningQuestion(this)) {
            m_paramWidget->setParam(param);
            return;
        }
    }
    // if state is TError or unknown enum value...
    else if (state != TConfigParam::TState::TOk && state != TConfigParam::TState::TInfo) {

        if(state != TConfigParam::TState::TError) {
            qWarning("An unknown value was encuntered as config param state!");
        }

        m_paramWidget->setParam(param);
        TDialog::paramValueErrorMessage(this);
        return;
    }

    QDialog::accept();
}

TScenarioConfigParamDialog::TScenarioConfigParamDialog(QString acceptText, QString title, TScenarioItem * item, QWidget * parent)
    : QDialog(parent), m_item(item)
{
    setWindowTitle(title);
    setMinimumSize(500, 400);

    TConfigParam param = m_item->getParams();
    m_originalParams = param;

    m_paramWidget = new TConfigParamWidget(param);

    // evaluate validity immediately on widget open
    m_paramWidget->setParam(m_paramWidget->param());  

    if(item->getConfigWindowSize() != QSize(0, 0)) {
        resize(item->getConfigWindowSize());
    }

    parent->connect(m_paramWidget, &TConfigParamWidget::inputValueChanged, this, &TScenarioConfigParamDialog::tryUpdateParams);

    QPushButton * leftButton = new QPushButton(tr("Update available options"));
    parent->connect(leftButton, &QPushButton::clicked, this, &TScenarioConfigParamDialog::updateParams);

    QDialogButtonBox * rightButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    rightButtonBox->button(QDialogButtonBox::Ok)->setText(acceptText);

    parent->connect(rightButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    parent->connect(rightButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(leftButton);
    buttonLayout->addWidget(rightButtonBox);

    QVBoxLayout * dialogLayout = new QVBoxLayout;
    dialogLayout->addWidget(m_paramWidget);
    dialogLayout->addLayout(buttonLayout);

    setLayout(dialogLayout);
}

void TScenarioConfigParamDialog::updateParams()
{
    m_item->updateParams(true);
    m_paramWidget->setParam(m_item->getParams());
}

void TScenarioConfigParamDialog::tryUpdateParams()
{
    TConfigParam newParams = m_paramWidget->param();
    if(!m_item->shouldUpdateParams(newParams)) {
        return;
    }

    m_paramWidget->setParam(m_item->setParams(newParams));
}

void TScenarioConfigParamDialog::accept()
{
    TConfigParam param = m_item->setParams(m_paramWidget->param());

    TConfigParam::TState state = param.getState(true);

    if (state == TConfigParam::TState::TWarning) {
        if (!TDialog::paramWarningQuestion(this)) {
            m_paramWidget->setParam(param);
            return;
        }
    }
    // if state is TError or unknown enum value...
    else if (state != TConfigParam::TState::TOk && state != TConfigParam::TState::TInfo) {

        if(state != TConfigParam::TState::TError) {
            qWarning("An unknown value was encuntered as config param state!");
        }

        m_paramWidget->setParam(param);
        TDialog::paramValueErrorMessage(this);
        return;
    }

    QDialog::accept();
}

void TScenarioConfigParamDialog::reject()
{
    m_item->setParams(m_originalParams);
    QDialog::reject();
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

TInitAnalDeviceDialog::TInitAnalDeviceDialog(TAnalDeviceModel * device, QWidget * parent)
    : TConfigParamDialog(tr("Initialize"), tr("Initialize Analytical Device"), device, true, parent)
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

TLoadingDialog::TLoadingDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setModal(true);
    setAttribute(Qt::WA_DeleteOnClose, false);

    QVBoxLayout * layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    QLabel * imageLabel = new QLabel(this);
    QPixmap pixmap(":/icons/tracexpert64.png");
    imageLabel->setPixmap(pixmap);
    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel);

    m_label = new QLabel("Please wait...", this);
    layout->addWidget(m_label);

    resize(250, 80);
}

TLoadingDialog* TLoadingDialog::showDialog(QWidget *parent, const QString &text)
{
    auto *dialog = new TLoadingDialog(parent);

    if (dialog->m_label)
        dialog->m_label->setText(text);

    if(parent) {
        QRect geom = parent->window()->geometry();
        dialog->move(geom.center() - QPoint(dialog->width()/2, dialog->height()/2));
    }

    dialog->show();
    QCoreApplication::processEvents();

    return dialog;
}

void TLoadingDialog::closeAndDeleteLater()
{
    hide();
    deleteLater();
}
