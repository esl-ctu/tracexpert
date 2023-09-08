#ifndef TCOMPONENTCONTAINER_H
#define TCOMPONENTCONTAINER_H

#include "tpluginunitcontainer.h"
#include "tcomponentmodel.h"

class TComponentContainer : public TPluginUnitContainer
{
    Q_OBJECT

public:
    explicit TComponentContainer(TProjectModel * parent);

    int unitCount() const override;
    TComponentModel * unit(int index) const override;

    void addComponent(TComponentModel * unit);

    QString name() const override;

private:
    QList<TComponentModel *> m_components;
};

#endif // TCOMPONENTCONTAINER_H
