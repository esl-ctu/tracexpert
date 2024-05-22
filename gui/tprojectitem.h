#ifndef TPROJECTITEM_H
#define TPROJECTITEM_H

#include <QList>
#include <QtXml/QDomDocument>

class TProjectModel;

class TProjectItem
{
public:
    explicit TProjectItem(TProjectModel * model, TProjectItem * parent);
    virtual ~TProjectItem();

    enum Status { None, Unavailable, Uninitialized, Initialized };

    static QVariant statusText(Status status);
    static QVariant statusIcon(Status status);

    virtual int childrenCount() const = 0;
    virtual TProjectItem * child(int row) const = 0;
    virtual QString name() const = 0;
    virtual Status status() const = 0;

    int row() const;

    TProjectItem * parent();
    TProjectModel * model();

    QString typeName() const;

    virtual bool toBeSaved() const;
    virtual QDomElement save(QDomDocument & document) const;

protected:
    void beginInsertChild(int childRow);
    void endInsertChild();
    void beginRemoveChild(int childRow);
    void endRemoveChild();
    void itemDataChanged();

    QString m_typeName = "unknown";

protected:
    TProjectModel * m_model;
    TProjectItem * m_parent;
};

#endif // TPROJECTITEM_H
