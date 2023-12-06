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

    bool init() override;
    bool deInit() override;

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
