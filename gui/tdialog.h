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

protected:
    static bool question(QWidget * parent, const QString & title, const QString & text);
    static void criticalMessage(QWidget * parent, const QString & title, const QString & text);
};

class TConfigParamDialog : public QDialog
{
public:
    explicit TConfigParamDialog(QString acceptText, QString title, QWidget * parent = nullptr);

    virtual TConfigParam param() = 0;
    virtual TConfigParam setParam() = 0;

    virtual void accept();

protected:
    TConfigParamWidget * m_paramWidget;
};

class TInitComponentDialog : public TConfigParamDialog
{
public:
    explicit TInitComponentDialog(TComponentModel * component, QWidget * parent = nullptr);

    virtual TConfigParam param();
    virtual TConfigParam setParam();

private:
    TComponentModel * m_component;
};

class TComponentSettingsDialog : public TConfigParamDialog
{
public:
    explicit TComponentSettingsDialog(TComponentModel * component, QWidget * parent = nullptr);

    virtual TConfigParam param();
    virtual TConfigParam setParam();

private:
    TComponentModel * m_component;
};

class TInitIODeviceDialog : public TConfigParamDialog
{
public:
    explicit TInitIODeviceDialog(TIODeviceModel * device, QWidget * parent = nullptr);

    virtual TConfigParam param();
    virtual TConfigParam setParam();

private:
    TIODeviceModel * m_IODevice;
};

class TInitScopeDialog : public TConfigParamDialog
{
public:
    explicit TInitScopeDialog(TScopeModel * scope, QWidget * parent = nullptr);

    virtual TConfigParam param();
    virtual TConfigParam setParam();

private:
    TScopeModel * m_scope;
};

#endif // TDIALOG_H
