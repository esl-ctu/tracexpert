#ifndef TSCENARIOITEMPORT_H
#define TSCENARIOITEMPORT_H

#include <QString>
#include <QList>
#include <QDataStream>

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

public:
    enum class TItemPortType {
        TFlowPort = 0,
        TDataPort = 1
    };

    enum class TItemPortDirection {
        TInputPort = 0,
        TOutputPort = 1
    };

    TScenarioItemPort(TScenarioItem * parentItem) : m_parentItem(parentItem) { }

    TScenarioItemPort(const QString &name, TItemPortType type, TItemPortDirection direction, TScenarioItem * parentItem, const QString &displayName = QString(), const QString &description = QString()):
        m_name(name),
        m_type(type),
        m_direction(direction),
        m_parentItem(parentItem),
        m_displayName(displayName),
        m_description(description)
    { }

    TScenarioItemPort(const TScenarioItemPort &x):
        m_name(x.m_name),
        m_type(x.m_type),
        m_direction(x.m_direction),
        m_parentItem(nullptr),
        m_displayName(x.m_displayName),
        m_description(x.m_description)
    { }

    TScenarioItemPort & operator=(const TScenarioItemPort &x){
        if(&x != this){
            m_name = x.m_name;
            m_type = x.m_type;
            m_direction = x.m_direction;
            m_parentItem = nullptr;
            m_displayName = x.m_displayName;
            m_description = x.m_description;
        }
        return *this;
    }

    bool operator==(const TScenarioItemPort &x) const {
        return (m_name == x.m_name && m_parentItem == x.m_parentItem);
    }

    friend QDataStream & operator<<(QDataStream &out, const TScenarioItemPort &x){
        out << QString(TSCENARIOITEMPORTVER);
        out << x.m_serializationId;
        out << x.m_name;
        out << x.m_type;
        out << x.m_direction;
        out << x.m_displayName;
        out << x.m_description;
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TScenarioItemPort &x){
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TSCENARIOITEMPORTVER))){
            in >> x.m_serializationId;
            in >> x.m_name;
            in >> x.m_type;
            in >> x.m_direction;
            in >> x.m_displayName;
            in >> x.m_description;
        } else {
            qCritical("Failed deserializing TScenarioItemPort: Wrong version or wrong data.");
        }
        return in;
    }

    const QString & getName() const {
        return m_name;
    }

    const QString & getDescription() const {
        return m_description;
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

    const QString & getDisplayName() const {
        return m_displayName;
    }

    void setName(const QString & value) {
        m_name = value;
    }

    void setDescription(const QString & value) {
        m_description = value;
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

    void setDisplayName(const QString & value) {
        m_displayName = value;
    }

protected:
    QString m_name;
    TItemPortType m_type;
    TItemPortDirection m_direction;

    uint m_serializationId;

    TScenarioItem * m_parentItem;

    QString m_displayName;
    QString m_description;
};


#endif // TSCENARIOITEMPORT_H
