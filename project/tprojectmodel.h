#ifndef TPROJECTMODEL_H
#define TPROJECTMODEL_H

#include <QAbstractItemModel>

#include "../scenario/tscenariocontainer.h"
#include "../pluginunit/component/tcomponentcontainer.h"
#include "../protocol/tprotocolcontainer.h"
#include "../protocol/tprotocolmodel.h"
#include "../scenario/tscenariomodel.h"

class TProjectModel : public QAbstractItemModel, public TProjectItem
{
    Q_OBJECT

public:
    explicit TProjectModel(QObject * parent = nullptr);
    ~TProjectModel();

    TComponentContainer * componentContainer();
    TProtocolContainer * protocolContainer();
    TScenarioContainer * scenarioContainer();

    QVariant data(const QModelIndex & index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex & index) const override;
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex & parent = QModelIndex()) const override;

    void emitDataChanged(const QModelIndex &topLeft, const QModelIndex & bottomRight);

    int childrenCount() const override;
    TProjectItem * child(int row) const override;
    QString name() const override;
    Status status() const override;

    void load(QDomElement * element);

private:
    void appendComponent(TPlugin * plugin, QDomElement * element = nullptr);

    void loadPlugins();
    void loadComponents(QDomElement * element);
    void loadComponent(QDomElement * element);

    void unloadComponents();
    
    void loadProtocols(QDomElement * element);
    void loadProtocol(QDomElement * element);

    void loadScenarios(QDomElement * element);
    void loadScenario(QDomElement * element);

    TComponentContainer * m_componentContainer;
    TProtocolContainer * m_protocolContainer;
    TScenarioContainer * m_scenarioContainer;

    friend class TProjectItem;

signals:
    void IODeviceInitialized(TIODeviceModel * IODevice);
    void IODeviceDeinitialized(TIODeviceModel * IODevice);

    void scopeInitialized(TScopeModel * scope);
    void scopeDeinitialized(TScopeModel * scope);

    void analDeviceInitialized(TAnalDeviceModel * analDevice);
    void analDeviceDeinitialized(TAnalDeviceModel * analDevice);

    void protocolManagerRequested();
    void protocolEditorRequested(TProtocolModel * protocol);

    void scenarioManagerRequested();
    void scenarioEditorRequested(TScenarioModel * scenario);
};

#endif // TPROJECTMODEL_H
