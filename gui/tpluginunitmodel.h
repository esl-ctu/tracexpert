#ifndef TPLUGINUNITMODEL_H
#define TPLUGINUNITMODEL_H

#include <QObject>

#include "tcommon.h"
#include "tconfigparam.h"
#include "tprojectitem.h"

class TPluginUnitContainer;

class TPluginUnitModel : public QObject, public virtual TProjectItem
{
    Q_OBJECT

public:
    explicit TPluginUnitModel(TCommon * unit, QObject * parent = nullptr);

    virtual QString name() const;
    virtual QString info() const;

    bool isInit() const;
    virtual bool init();
    virtual bool deInit();

    virtual TConfigParam preInitParams() const;
    virtual TConfigParam postInitParams() const;
    virtual TConfigParam setPreInitParams(const TConfigParam & param);
    virtual TConfigParam setPostInitParams(const TConfigParam & param);

protected:
    TCommon * m_unit;

    QString m_name;
    QString m_info;

    TConfigParam m_preInitParam;
    TConfigParam m_postInitParam;

    bool m_isInit;
    bool m_isManual;
};

#endif // TPLUGINUNITMODEL_H
