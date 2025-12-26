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

#ifndef TDEVICEWIZARD_H
#define TDEVICEWIZARD_H

#include <QWizard>
#include <QListWidget>
#include <QTableWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QLabel>

#include "widgets/tconfigparamwidget.h"
#include "pluginunit/component/tcomponentcontainer.h"
#include "pluginunit/component/tcomponentmodel.h"
#include "pluginunit/tpluginunitcontainerview.h"

class TDeviceWizard : public QWizard
{
    Q_OBJECT

public:
    explicit TDeviceWizard(TComponentContainer * componentContainer, QWidget * parent = nullptr, TComponentModel * component = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    enum Pages {Page_SelectComponent, Page_InitComponent, Page_PostInitComponent, Page_SelectDevice, Page_InitDevice, Page_Final};

private:
    TComponentContainer * m_componentContainer;
    TComponentModel * m_selectedComponent = nullptr;
    TDeviceModel * m_selectedDevice = nullptr;

    friend class TDeviceWizardPage;
};

class TDeviceWizardPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit TDeviceWizardPage(TDeviceWizard * parent);

protected:
    bool initComponent(TConfigParam * param = nullptr);
    bool deInitComponent();

    bool initDevice(TConfigParam * param = nullptr);
    bool deInitDevice();

    bool checkParam(const TConfigParam & param);

    TComponentContainer * m_componentContainer;
    TComponentModel * & m_selectedComponent;
    TDeviceModel * & m_selectedDevice;
};


class TSelectComponentWizardPage : public TDeviceWizardPage
{
    Q_OBJECT

public:
    explicit TSelectComponentWizardPage(TDeviceWizard *parent);

    void initializePage();
    bool isComplete() const;
    int nextId() const;
    bool validatePage();

private slots:
    void selectComponent();

private:
    TPluginUnitContainerView * m_componentListWidget;
    QCheckBox * m_reInitCheckBox;
};

class TInitComponentWizardPage : public TDeviceWizardPage
{
    Q_OBJECT

public:
    explicit TInitComponentWizardPage(TDeviceWizard *parent);

    void initializePage();
    bool isComplete() const;
    int nextId() const;
    bool validatePage();

private:
    TConfigParamWidget * m_componentPreInitParamWidget;
};

class TPostInitComponentWizardPage : public TDeviceWizardPage
{
    Q_OBJECT

public:
    explicit TPostInitComponentWizardPage(TDeviceWizard *parent);

    void initializePage();
    bool isComplete() const;
    int nextId() const;
    bool validatePage();

private:
    TConfigParamWidget * m_componentPostInitParamWidget;
};

class TSelectDeviceWizardPage : public TDeviceWizardPage
{
    Q_OBJECT

public:
    explicit TSelectDeviceWizardPage(TDeviceWizard *parent);

    void initializePage();
    bool isComplete() const;
    int nextId() const;
    bool validatePage();

protected slots:
    void selectIODevice();
    void selectScope();
    void selectAnalDevice();

    bool addIODevice();
    bool addScope();
    bool addAnalDevice();

private:
    TPluginUnitContainerView * m_IODeviceListWidget;
    TPluginUnitContainerView * m_scopeListWidget;
    TPluginUnitContainerView * m_analDeviceListWidget;
    QPushButton * m_addIOButton;
    QGroupBox * m_IOGroupBox;
    QPushButton * m_addScopeButton;
    QGroupBox * m_scopeGroupBox;
    QPushButton * m_addAnalButton;
    QGroupBox * m_analGroupBox;
};

class TInitDeviceWizardPage : public TDeviceWizardPage
{
    Q_OBJECT

public:
    explicit TInitDeviceWizardPage(TDeviceWizard *parent);

    void initializePage();
    bool isComplete() const;
    int nextId() const;
    bool validatePage();

private:
    TConfigParamWidget * m_devicePreInitParamWidget;
};

class TFinalWizardPage : public TDeviceWizardPage
{
    Q_OBJECT

public:
    explicit TFinalWizardPage(TDeviceWizard *parent);

    void initializePage();
    bool isComplete() const;
    int nextId() const;
    bool validatePage();
};

#endif // TDEVICEWIZARD_H
