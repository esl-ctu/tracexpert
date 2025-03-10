#ifndef TPROTOCOLCONTAINER_H
#define TPROTOCOLCONTAINER_H

#include <QAbstractTableModel>
#include "tprotocolmodel.h"

class TProtocolContainer : public QAbstractTableModel, public TProjectItem {
    Q_OBJECT

public:
    explicit TProtocolContainer(TProjectModel * parent);
    ~TProtocolContainer();

    TProtocol getByName(const QString &name, bool *ok = nullptr) const;
    int getIndexByName(const QString &name, bool *ok = nullptr) const;

    int count() const;
    const TProtocol & at(int index) const;

    bool add(TProtocolModel * protocolModel);
    bool add(const TProtocol & protocol);
    bool update(int index, const TProtocol & protocol);
    bool remove(int index);

    // methods for QAbstractTableModel - to be able to show Protocols in a table
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // methods for TProjectItem - to be able to show Protocols in the Project view
    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    Status status() const override;

signals:
    void protocolsUpdated();

private:
    using QAbstractTableModel::sort;
    void sort();

    QList<TProtocolModel *> m_protocolModels;
};


#endif // TPROTOCOLCONTAINER_H
