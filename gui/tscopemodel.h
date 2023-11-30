#ifndef TSCOPEMODEL_H
#define TSCOPEMODEL_H

#include <QObject>

#include "tpluginunitmodel.h"
#include "tscope.h"

class TScopeContainer;

class TScopeModel : public TPluginUnitModel
{
    Q_OBJECT

public:
    explicit TScopeModel(TScope * scope, TScopeContainer * parent);
    
    QString name() const override;
    QString info() const override;

    bool init() override;
    bool deInit() override;
    
    TConfigParam preInitParams() const override;
    TConfigParam postInitParams() const override;
    TConfigParam setPreInitParams(const TConfigParam & param) override;
    TConfigParam setPostInitParams(const TConfigParam & param) override;

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QVariant status() const override;

signals:
    void initialized(TScopeModel * scope);
    void deinitialized(TScopeModel * scope);

private:
    TScope * m_scope;
};

#endif // TSCOPEMODEL_H
