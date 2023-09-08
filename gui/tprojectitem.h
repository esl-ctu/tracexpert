#ifndef TPROJECTITEM_H
#define TPROJECTITEM_H

#include <QList>

class TProjectModel;

class TProjectItem
{
public:
    explicit TProjectItem(TProjectModel * model, TProjectItem * parent);
    virtual ~TProjectItem();

    virtual int childrenCount() const = 0;
    virtual TProjectItem * child(int row) const = 0;
    virtual QString name() const = 0;
    virtual QVariant status() const = 0;

    int row() const;

    TProjectItem * parent();
    TProjectModel * model();

protected:
    void beginInsertChild(int childRow);
    void endInsertChild();
    void itemDataChanged();

private:
    TProjectModel * m_model;
    TProjectItem * m_parent;
};

#endif // TPROJECTITEM_H
