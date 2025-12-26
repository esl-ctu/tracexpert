// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Vojtěch Miškovský (initial author)
// Adam Švehla

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

    virtual QDomElement save(QDomDocument & document) const override;
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
