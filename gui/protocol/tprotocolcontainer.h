#ifndef TPROTOCOLCONTAINER_H
#define TPROTOCOLCONTAINER_H

#include "tabstracttablemodel.h"
#include "tprotocolmodel.h"

class TProtocolContainer : public TAbstractTableModel<TProtocolModel *>, public TProjectItem {
    Q_OBJECT

public:
    explicit TProtocolContainer(TProjectModel * parent);

    TProtocol getProtocolByName(const QString &name, bool *ok = nullptr);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    QVariant status() const override;
};


#endif // TPROTOCOLCONTAINER_H
