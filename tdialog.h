// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Vojtěch Miškovský (initial author)
// Adam Švehla
// Petr Socha

#ifndef TDIALOG_H
#define TDIALOG_H

#include <QDialog>
#include <QLabel>

#include "scenario/tscenarioconfigwidget.h"
#include "widgets/tconfigparamwidget.h"
#include "pluginunit/component/tcomponentmodel.h"
#include "pluginunit/io/tiodevicemodel.h"
#include "scenario/tscenarioitem.h"
#include "pluginunit/scope/tscopemodel.h"
#include "pluginunit/anal/tanaldevicemodel.h"

class TDialog
{
public:
    static bool addDeviceDialog(QWidget * parent, QString & name, QString & info);
    static bool renameDeviceDialog(QWidget * parent, QString & name, QString & info);

    static bool exportImageDimensionsDialog(QWidget * parent, uint &width, uint &height);

    static bool paramErrorQuestion(QWidget * parent);
    static bool paramWarningQuestion(QWidget * parent);
    static bool componentReinitQuestion(QWidget * parent);
    static bool componentDeinitQuestion(QWidget * parent);
    static bool deviceReinitQuestion(QWidget * parent);
    static bool deviceOpenQuestion(QWidget * parent);
    static bool deviceDeinitQuestion(QWidget * parent);
    static bool deviceRemoveQuestion(QWidget * parent);

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
    static void paramValueErrorMessage(QWidget * parent);

    static void protocolMessageCouldNotBeFormed(QWidget * parent);

    static bool itemsRemoveQuestion(QWidget * parent, QString removedObjectName = QString());

    static bool closeConfirmation(QWidget * parent, QString closedObjectName = QString());
    static bool scenarioTerminationConfirmation(QWidget * parent);

protected:
    static bool question(QWidget * parent, const QString & title, const QString & text);
    static void criticalMessage(QWidget * parent, const QString & title, const QString & text);
};

class TScenarioConfigParamDialog : public QDialog
{
public:
    explicit TScenarioConfigParamDialog(QString acceptText, QString title, TScenarioItem * item, QWidget * parent = nullptr);
    void tryUpdateParams();
    void updateParams();

protected:
    void accept() override;
    void reject() override;

private:
    TScenarioItem * m_item;
    TScenarioConfigParamWidget * m_scenarioParamWidget;
    TConfigParam m_originalParams;
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

class TInitAnalDeviceDialog : public TConfigParamDialog
{
public:
    explicit TInitAnalDeviceDialog(TAnalDeviceModel * device, QWidget * parent = nullptr);
};

class TPluginUnitInfoDialog : public QDialog
{
public:
    explicit TPluginUnitInfoDialog(TPluginUnitModel * unit, QWidget * parent = nullptr);
};

class TLoadingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TLoadingDialog(QWidget *parent = nullptr);

    static TLoadingDialog* showDialog(QWidget *parent, const QString &text = "Please wait...");

    void closeAndDeleteLater();
private:
    QLabel * m_label;
};

#endif // TDIALOG_H
