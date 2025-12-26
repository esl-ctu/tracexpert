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
// Adam Švehla (initial author)

#ifndef TSCENARIOITEMPORT_H
#define TSCENARIOITEMPORT_H

#include <QString>
#include <QSet>
#include <QDataStream>
#include <QCoreApplication>

#define TSCENARIOITEMPORTVER "cz.cvut.fit.TraceXpert.TScenarioItemPort/0.1"

/*!
 * \brief The TScenario class represents a scenario item port.
 *
 * The class represents a scenario item port.
 * The whole class can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 *
 * The item port has a name, type, direction, display name and description.
 *
 */
class TScenarioItem;

class TScenarioItemPort {
    Q_DECLARE_TR_FUNCTIONS(TScenarioItemPort)

public:
    enum class TItemPortType {
        TConnectionPort = 0,
        TFlowPort = 1,
        TDataPort = 2,
    };

    enum class TItemPortDirection {
        TInputPort = 0,
        TOutputPort = 1
    };

    TScenarioItemPort(TScenarioItem * parentItem) : m_parentItem(parentItem), m_connectedPorts() { }

    TScenarioItemPort(
        const QString &name,
        TItemPortType type,
        TItemPortDirection direction,
        TScenarioItem * parentItem,
        const QString &labelText = QString(),
        const QString &description = QString(),
        const QString &dataTypeHint = QString()
    ):
        m_name(name),
        m_description(description),
        m_dataTypeHint(dataTypeHint),
        m_labelText(labelText),
        m_type(type),
        m_direction(direction),
        m_parentItem(parentItem),
        m_connectedPorts()
    { }

    TScenarioItemPort(const TScenarioItemPort &x):
        m_name(x.m_name),
        m_description(x.m_description),
        m_dataTypeHint(x.m_dataTypeHint),
        m_labelText(x.m_labelText),
        m_type(x.m_type),
        m_direction(x.m_direction),
        m_parentItem(nullptr),
        m_connectedPorts()
    { }

    TScenarioItemPort & operator=(const TScenarioItemPort &x){
        if(&x != this){
            m_name = x.m_name;
            m_description = x.m_description;
            m_dataTypeHint = x.m_dataTypeHint;
            m_labelText = x.m_labelText;
            m_type = x.m_type;
            m_direction = x.m_direction;
            m_parentItem = nullptr;
        }
        return *this;
    }

    bool operator==(const TScenarioItemPort *x) const {
        return (m_name == x->m_name && m_parentItem == x->m_parentItem);
    }

    bool operator==(const TScenarioItemPort &x) const {
        return (m_name == x.m_name && m_parentItem == x.m_parentItem);
    }

    friend QDataStream & operator<<(QDataStream &out, const TScenarioItemPort &x){
        out << QString(TSCENARIOITEMPORTVER);
        out << x.m_serializationId;
        out << x.m_name;
        out << x.m_description;
        out << x.m_dataTypeHint;
        out << x.m_labelText;
        out << x.m_type;
        out << x.m_direction;
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TScenarioItemPort &x){
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TSCENARIOITEMPORTVER))){
            in >> x.m_serializationId;
            in >> x.m_name;
            in >> x.m_description;
            in >> x.m_dataTypeHint;
            in >> x.m_labelText;
            in >> x.m_type;
            in >> x.m_direction;
        } else {
            qCritical("Failed deserializing TScenarioItemPort: Wrong version or wrong data.");
            throw tr("Failed deserializing scenario item port: Wrong version or wrong data.");
        }
        return in;
    }

    const QString & getName() const {
        return m_name;
    }

    const QString & getDescription() const {
        return m_description;
    }

    const QString & getDataTypeHint() const {
        return m_dataTypeHint;
    }

    const QString & getLabelText() const {
        return m_labelText;
    }

    TItemPortType getType() const {
        return m_type;
    }

    TItemPortDirection getDirection() const {
        return m_direction;
    }

    uint getSerializationId() {
        return m_serializationId;
    }

    TScenarioItem * getParentItem() {
        return m_parentItem;
    }

    void setName(const QString & value) {
        m_name = value;
    }

    void setDescription(const QString & value) {
        m_description = value;
    }

    void setDataTypeHint(const QString & value) {
        m_dataTypeHint = value;
    }

    void setLabelText(const QString & value) {
        m_labelText = value;
    }

    void setType(TItemPortType value) {
        m_type = value;
    }

    void setDirection(TItemPortDirection value) {
        m_direction = value;
    }

    void setParentItem(TScenarioItem * parent) {
        m_parentItem = parent;
    }

    void setSerializationId(uint id) {
        m_serializationId = id;
    }    

    bool hasConnectedPort() const {
        return !m_connectedPorts.empty();
    }

    QSet<TScenarioItemPort *> getConnectedPorts() const {
        return m_connectedPorts;
    }

    void clearConnectedPorts() {
        m_connectedPorts.clear();
    }

    void addConnectedPort(TScenarioItemPort * itemPort) {
        m_connectedPorts.insert(itemPort);
    }

    void removeConnectedPort(TScenarioItemPort * itemPort) {
        m_connectedPorts.remove(itemPort);
    }

protected:
    QString m_name;
    QString m_description;
    QString m_dataTypeHint;
    QString m_labelText;

    TItemPortType m_type;
    TItemPortDirection m_direction;

    uint m_serializationId;

    TScenarioItem * m_parentItem;  

    // NOT copied, NOT serialized,
    // available both during editing and execution
    // TScenarioItems that are connected through this port
    QSet<TScenarioItemPort*> m_connectedPorts;
};

#endif // TSCENARIOITEMPORT_H
