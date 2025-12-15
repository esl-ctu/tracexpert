#ifndef TPROJECTUNIT_H
#define TPROJECTUNIT_H

#include <QString>

class TProjectUnit
{
public:
    virtual ~TProjectUnit() {};

    virtual const QString & name() const = 0;
    virtual const QString & description() const = 0;

    virtual void setName(const QString & name) = 0;
    virtual void setDescription(const QString & name) = 0;

    virtual TProjectUnit * copy() const = 0;

    virtual void serialize(QDataStream &out) const = 0;
    virtual void deserialize(QDataStream &in) = 0;

    static TProjectUnit * instantiate(const QString & typeName);
    static TProjectUnit * deserialize(const QString & typeName, QDataStream &in);
};

#endif // TPROJECTUNIT_H
