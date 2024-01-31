#ifndef TPROTOCOLMODEL_H
#define TPROTOCOLMODEL_H

#include <QObject>
#include "tprotocol.h"
#include "tprojectitem.h"

class TProtocolContainer;

class TProtocolModel : public QObject, public TProjectItem {
    Q_OBJECT

public:
    explicit TProtocolModel(TProtocol protocol, TProtocolContainer * parent);

    TProtocol protocol() const { return m_protocol; }

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    QVariant status() const override;

private:
    TProtocol m_protocol;

};



#endif // TPROTOCOLMODEL_H
