#ifndef TPLUGINUNITMODEL_H
#define TPLUGINUNITMODEL_H

#include <QObject>

#include "tconfigparam.h"
#include "tprojectitem.h"

class TPluginUnitContainer;

class TPluginUnitModel : public QObject, public virtual TProjectItem
{
    Q_OBJECT

public:
    explicit TPluginUnitModel(QObject * parent = nullptr);

    virtual QString name() const;
    virtual QString info() const;

    bool isInit() const;
    virtual bool init() = 0;
    virtual bool deInit() = 0;

    virtual TConfigParam preInitParams() const = 0;
    virtual TConfigParam postInitParams() const = 0;
    virtual TConfigParam setPreInitParams(const TConfigParam & param) = 0;
    virtual TConfigParam setPostInitParams(const TConfigParam & param) = 0;

protected:
    QString m_name;
    QString m_info;

    TConfigParam m_preInitParam;
    TConfigParam m_postInitParam;

    bool m_isInit;
    bool m_isManual;
};

#endif // TPLUGINUNITMODEL_H
