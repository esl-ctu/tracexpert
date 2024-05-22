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
    explicit TPluginUnitModel(TCommon * unit, QObject * parent = nullptr, bool manual = false);

    virtual QString name() const override;
    virtual QString info() const;

    virtual bool init();
    virtual bool deInit();

    virtual bool remove();

    bool isInit() const;
    bool initWhenAvailable() const;
    bool isAvailable() const;
    bool isManual() const;

    virtual TConfigParam preInitParams() const;
    virtual TConfigParam postInitParams() const;
    virtual TConfigParam setPreInitParams(const TConfigParam & param);
    virtual TConfigParam setPostInitParams(const TConfigParam & param);

    virtual bool toBeSaved() const override;
    virtual QDomElement save(QDomDocument & document) const override;
    virtual void load(QDomElement * element);

    virtual void bind(TCommon * unit);
    virtual void release();

    Status status() const override;

protected:
    TCommon * m_unit;

    QString m_name;
    QString m_info;

    TConfigParam m_preInitParam;
    TConfigParam m_postInitParam;

    bool m_isInit;
    bool m_wasInit;
    bool m_initWhenAvailable;
    bool m_isManual;

private:
    QByteArray saveParam(const TConfigParam & param) const;
    TConfigParam loadParam(const QByteArray & array) const;
};

#endif // TPLUGINUNITMODEL_H
