#ifndef TSCENARIO_H
#define TSCENARIO_H

#include <QString>
#include <QList>
#include <QDataStream>
#include <QHash>

#include "tscenarioitem.h"
#include "tscenarioconnection.h"

#define TSCENARIOVER "cz.cvut.fit.TraceXpert.TScenario/0.1"

/*!
 * \brief The TScenario class represents a scenario that can be executed.
 *
 * The class represents a scenario that can be executed. It is a container for scenario items and connections between them.
 * The scenario can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 *
 * It includes methods for adding and removing items and connections.
 * It includes simple scenario validation method and keeps state of the scenario.
 *
 */
class TScenario {

public:

    enum class TState {
        TOk = 0,
        TInfo = 10,
        TWarning = 20,
        TError = 30
    };

    TScenario() { }

    ~TScenario() {
        qDeleteAll(m_items);
        m_items.clear();
        qDeleteAll(m_connections);
        m_connections.clear();
    }

    TScenario(const QString &name, const QString &description):
        m_name(name),
        m_description(description),
        m_items(),
        m_connections()
    { }

    TScenario(const QString &name, const QString &description, QList<TScenarioItem *> items, QList<TScenarioConnection *> connections):
        m_name(name),
        m_description(description),
        m_items(items),
        m_connections(connections)
    { }

    TScenario(const TScenario &x):
        m_name(x.m_name),
        m_description(x.m_description)
    {
        deepCopy(x);
    }

    TScenario & operator=(const TScenario &x) {
        if(&x != this){
            m_name = x.m_name;
            m_description = x.m_description;

            // delete previous items and connections of this object...
            qDeleteAll(m_items);
            m_items.clear();

            qDeleteAll(m_connections);
            m_connections.clear();

            deepCopy(x);
        }
        return *this;
    }

    bool operator==(const TScenario &x) const {
        return (m_name == x.m_name);
    }

    bool operator==(const QString &x) const {
        return (m_name == x);
    }

    friend QDataStream & operator<<(QDataStream &out, const TScenario &x) {
        out << QString(TSCENARIOVER);
        out << x.m_name;
        out << x.m_description;

        uint nextItemPortId = 0;

        out << x.m_items.size();
        for(TScenarioItem * e : x.m_items) {
            for(TScenarioItemPort * p : e->getItemPorts()) {
                p->setSerializationId(nextItemPortId++);
            }

            out << e->itemClass();
            out << *e;
        }


        out << x.m_connections.size();
        for(TScenarioConnection * e : x.m_connections) {
            out << *e;
        }
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TScenario &x){
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TSCENARIOVER))){
            qDeleteAll(x.m_items);
            x.m_items.clear();

            qDeleteAll(x.m_connections);
            x.m_connections.clear();

            in >> x.m_name;
            in >> x.m_description;

            QHash<uint, TScenarioItemPort *> portMap;

            qsizetype itemCount;
            in >> itemCount;
            for(qsizetype i = 0; i < itemCount; i++) {
                int itemClass;
                in >> itemClass;

                TScenarioItem * item = TScenarioItem::createScenarioItemByClass(itemClass);

                if(!item) {
                    qCritical("Failed deserializing TScenario: Cannot instantiate correct scenario item class.");
                    return in;
                }

                in >> item;
                x.m_items.append(item);

                for(TScenarioItemPort * p : item->getItemPorts()) {
                    portMap.insert(p->getSerializationId(), p);
                }
            }

            qsizetype connectionCount;
            in >> connectionCount;
            for(qsizetype i = 0; i < connectionCount; i++) {
                TScenarioItemPort * dummySourcePort = new TScenarioItemPort(nullptr);
                TScenarioItemPort * dummyTargetPort = new TScenarioItemPort(nullptr);
                TScenarioConnection * dummyConnection = new TScenarioConnection(dummySourcePort, dummyTargetPort);
                in >> *dummyConnection;

                TScenarioConnection * connection = new TScenarioConnection(
                    portMap.value(dummySourcePort->getSerializationId()),
                    portMap.value(dummyTargetPort->getSerializationId())
                );

                connection->setPreferredLineBreakCoord(dummyConnection->getPreferredLineBreakCoord());

                delete dummySourcePort;
                delete dummyTargetPort;
                delete dummyConnection;

                x.m_connections.append(connection);
            }
        } else {
            qCritical("Failed deserializing TScenario: Wrong version or wrong data.");
        }
        return in;
    }

    bool hasItem(TScenarioItem * param) {
        if(m_items.contains(param)){
            return true;
        }
        return false;
    }

    void addItem(TScenarioItem * param, bool *ok = nullptr) {
        if(m_items.contains(param)){
            if(ok != nullptr) *ok = false;
            return;
        }
        m_items.append(param);
        if(ok != nullptr) *ok = true;
    }

    void removeItem(TScenarioItem * param, bool *ok = nullptr) {
        qsizetype removed = m_items.removeAll(param);
        if(ok != nullptr) *ok = removed > 0;
    }

    void addConnection(TScenarioConnection * param, bool *ok = nullptr) {
        if(m_connections.contains(param)){
            if(ok != nullptr) *ok = false;
            return;
        }
        m_connections.append(param);
        if(ok != nullptr) *ok = true;
    }

    void removeConnection(TScenarioConnection * param, bool *ok = nullptr) {
        qsizetype removed = m_connections.removeAll(param);
        if(ok != nullptr) *ok = removed > 0;
    }

    /**
     * @brief Validates that the scenario has exactly one flow start and at least one flow end block.
     * @return Result of validation; bool.
     */
    bool validate() {
        int flowStartItems = 0;
        int flowEndItems = 0;

        for(TScenarioItem * item : m_items) {
            if(item->getType() == TScenarioItem::TItemAppearance::TFlowStart) {
                flowStartItems++;
            }

            if(item->getType() == TScenarioItem::TItemAppearance::TFlowEnd) {
                flowEndItems++;
            }
        }

        if(flowStartItems != 1) {
            setState(TState::TError, "The scenario needs exactly one flow start block.");
            return false;
        }

        if(flowEndItems < 1) {
            setState(TState::TError, "The scenario is missing a flow end block.");
            return false;
        }

        return true;
    }

    const QString & getName() const {
        return m_name;
    }

    const QString & getDescription() const {
        return m_description;
    }

    const QList<TScenarioItem *> getItems() const {
        return m_items;
    }

    const QList<TScenarioConnection *> getConnections() const {
        return m_connections;
    }

    void setName(const QString & value) {
        m_name = value;
    }

    void setDescription(const QString & value) {
        m_description = value;
    }

    TScenario::TState getState() const {
        return m_state;
    }

    const QString & getStateMessage() const {
        return m_stateMessage;
    }


    void setState(TState state) {
        m_state = state;
    }

    void setState(TState state, const QString &message) {
        m_state = state;
        m_stateMessage = message;
    }

    void resetState(bool includeItems = false) {
        m_state = TState::TOk;
        m_stateMessage = "";
        if(includeItems == true){
            for(int i = 0; i < m_items.count(); i++){
                m_items[i]->resetState();
            }
        }
    }

private:

    void deepCopy(const TScenario &x) {
        QHash<TScenarioItemPort *, TScenarioItemPort *> portTranslations;

        // deep copy items
        for(TScenarioItem * item : x.m_items) {
            TScenarioItem * newItem = (*item).copy();

            qsizetype itemPortCount = item->getItemPorts().count();
            for(qsizetype i = 0; i < itemPortCount; i++) {
                portTranslations.insert(item->getItemPorts().at(i), newItem->getItemPorts().at(i));
            }

            m_items.append(newItem);
        }

        // deep copy connections
        for(TScenarioConnection * connection : x.m_connections) {
            TScenarioConnection * connectionCopy = new TScenarioConnection(
                portTranslations.value(connection->getSourcePort()),
                portTranslations.value(connection->getTargetPort())
            );
            connectionCopy->setPreferredLineBreakCoord(connection->getPreferredLineBreakCoord());
            m_connections.append(connectionCopy);
        }
    }

    QString m_name;
    QString m_description;
    QList<TScenarioItem *> m_items;
    QList<TScenarioConnection *> m_connections;

    TState m_state;
    QString m_stateMessage;
};


#endif // TSCENARIO_H
