#ifndef TPROJECTMODEL_H
#define TPROJECTMODEL_H

#include <QAbstractItemModel>

#include "tcomponentcontainer.h"
#include "protocol/tprotocolcontainer.h"

class TProjectModel : public QAbstractItemModel, public TProjectItem
{
    Q_OBJECT

public:
    explicit TProjectModel(QObject * parent = nullptr);
    ~TProjectModel();

    TComponentContainer * componentContainer();
    TProtocolContainer * protocolContainer();

    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;

    void emitDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    QVariant status() const override;

private:
    void loadComponents();
    void loadProtocols();

    TComponentContainer * m_componentContainer;
    TProtocolContainer * m_protocolContainer;

    friend class TProjectItem;

signals:
    void IODeviceInitialized(TIODeviceModel * IODevice);
    void IODeviceDeinitialized(TIODeviceModel * IODevice);

    void scopeInitialized(TScopeModel * scope);
    void scopeDeinitialized(TScopeModel * scope);
};

#endif // TPROJECTMODEL_H
