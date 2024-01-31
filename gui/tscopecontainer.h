#ifndef TSCOPECONTAINER_H
#define TSCOPECONTAINER_H

#include "tpluginunitcontainer.h"
#include "tscopemodel.h"

class TComponentModel;

class TScopeContainer : public TPluginUnitContainer
{
    Q_OBJECT

public:
    explicit TScopeContainer(TComponentModel * parent);
    
    int count() const override;
    TScopeModel * at(int index) const override;

    void add(TScopeModel * unit);
    
    QString name() const override;

private:
    QList<TScopeModel *> m_scopes;
};

#endif // TSCOPECONTAINER_H
