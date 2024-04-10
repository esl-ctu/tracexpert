#ifndef TDIALOG_H
#define TDIALOG_H

#include <QDialog>

#include "tconfigparamwidget.h"
#include "tcomponentmodel.h"
#include "tiodevicemodel.h"
#include "tscopemodel.h"

class TDialog
{
public:
    static bool addDeviceDialog(QWidget * parent, QString & name, QString & info);

    static bool paramWarningQuestion(QWidget * parent);
    static bool componentReinitQuestion(QWidget * parent);
    static bool componentDeinitQuestion(QWidget * parent);
    static bool deviceReinitQuestion(QWidget * parent);

    static void componentInitFailedGeneralMessage(QWidget * parent);
    static void componentDeinitFailedGeneralMessage(QWidget * parent);

    static void deviceInitFailedGeneralMessage(QWidget * parent);
    static void deviceInitFailedNoSelectionMessage(QWidget * parent);
    static void deviceDeinitFailedGeneralMessage(QWidget * parent);
    static void deviceDeinitFailedNoSelectionMessage(QWidget * parent);

    static void deviceAddFailedGeneralMessage(QWidget * parent);

    static void deviceFailedBusyMessage(QWidget * parent);

    static void deviceReceiveFailedMessage(QWidget * parent);
    static void deviceSendFailedMessage(QWidget * parent);

    static void parameterValueEmpty(QWidget * parent, const QString & parameterName);
    static void parameterValueInvalid(QWidget * parent, const QString & parameterName);
    static void parameterValueNotUniqueMessage(QWidget * parent, const QString & parameterName);

    static void protocolMessageCouldNotBeFormed(QWidget * parent);

protected:
    static bool question(QWidget * parent, const QString & title, const QString & text);
    static void criticalMessage(QWidget * parent, const QString & title, const QString & text);
};

class TConfigParamDialog : public QDialog
{
public:
    explicit TConfigParamDialog(QString acceptText, QString title, TPluginUnitModel * unit, bool preInit, QWidget * parent = nullptr);

protected:
    void accept() override;

private:
    TPluginUnitModel * m_unit;
    TConfigParamWidget * m_paramWidget;
    bool m_preInit;
};

class TInitComponentDialog : public TConfigParamDialog
{
public:
    explicit TInitComponentDialog(TComponentModel * component, QWidget * parent = nullptr);
};

class TComponentSettingsDialog : public TConfigParamDialog
{
public:
    explicit TComponentSettingsDialog(TComponentModel * component, QWidget * parent = nullptr);
};

class TInitIODeviceDialog : public TConfigParamDialog
{
public:
    explicit TInitIODeviceDialog(TIODeviceModel * device, QWidget * parent = nullptr);
};

class TInitScopeDialog : public TConfigParamDialog
{
public:
    explicit TInitScopeDialog(TScopeModel * scope, QWidget * parent = nullptr);
};

class TPluginUnitInfoDialog : public QDialog
{
public:
    explicit TPluginUnitInfoDialog(TPluginUnitModel * unit, QWidget * parent = nullptr);
};

#endif // TDIALOG_H
