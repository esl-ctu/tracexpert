#ifndef TPROTOCOLMODEL_H
#define TPROTOCOLMODEL_H

#include <QObject>

#include "tprotocol.h"
#include "../project/tprojectitem.h"

class TProtocolContainer;

class TProtocolModel : public QObject, public TProjectItem {
    Q_OBJECT

public:
    TProtocolModel(TProtocolContainer * parent);
    TProtocolModel(TProtocol protocol, TProtocolContainer * parent);

    const TProtocol & protocol() const;
    void setProtocol(const TProtocol & protocol);

    // methods for TProjectItem - to be able to show Protocols in the Project view
    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    Status status() const override;

    bool toBeSaved() const override;
    QDomElement save(QDomDocument & document) const override;
    void load(QDomElement * element);

private:
    QByteArray saveMessages(const QList<TMessage> & messages) const;
    QList<TMessage> loadMessages(const QByteArray & array) const;

    TProtocol m_protocol;
};



#endif // TPROTOCOLMODEL_H
