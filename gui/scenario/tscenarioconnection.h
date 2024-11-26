#ifndef TSCENARIOCONNECTION_H
#define TSCENARIOCONNECTION_H

#include <QString>
#include <QList>
#include <QDataStream>

#include "tscenarioitemport.h"

#define TSCENARIOCONNECTIONVER "cz.cvut.fit.TraceXpert.TScenarioConnection/0.1"

/*!
 * \brief The TScenarioConnection class represents a scenario connection between two ports.
 *
 * The class represents a scenario connection between two ports.
 * The whole class can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 *
 * It includes methods for getting and setting source and target ports.
 * It includes a method for getting and setting the preferred "line break" coordinate
 * for purposes of drawing the physical connecting line in the user selected spot in the scenario scene.
 *
 */
class TScenarioConnection {

public:

    TScenarioConnection() { }

    TScenarioConnection(TScenarioItemPort * sourcePort, TScenarioItemPort * targetPort):
        m_sourcePort(sourcePort),
        m_targetPort(targetPort)
    { }

    TScenarioConnection(const TScenarioConnection &x):
        m_preferredLineBreakCoord(x.m_preferredLineBreakCoord),
        m_sourcePort(x.m_sourcePort),
        m_targetPort(x.m_targetPort)
    { }

    TScenarioConnection & operator=(const TScenarioConnection &x){
        if(&x != this){
            m_preferredLineBreakCoord = x.m_preferredLineBreakCoord;
            m_sourcePort = x.m_sourcePort;
            m_targetPort = x.m_targetPort;
        }
        return *this;
    }

    bool operator==(const TScenarioConnection &x) const {
        return (m_sourcePort == x.m_sourcePort && m_targetPort == x.m_targetPort);
    }

    friend QDataStream & operator<<(QDataStream &out, const TScenarioConnection &x){
        out << QString(TSCENARIOCONNECTIONVER);
        out << *x.m_sourcePort;
        out << *x.m_targetPort;
        out << x.m_preferredLineBreakCoord;
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TScenarioConnection &x){
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TSCENARIOCONNECTIONVER))){
            if(x.m_sourcePort == nullptr || x.m_targetPort == nullptr) {
                qCritical("Failed deserializing TScenarioConnection: source or target port are nullptr.");
                return in;
            }
            in >> *x.m_sourcePort;
            in >> *x.m_targetPort;
            in >> x.m_preferredLineBreakCoord;
        } else {
            qCritical("Failed deserializing TScenarioConnection: Wrong version or wrong data.");
        }
        return in;
    }

    TScenarioItemPort * getSourcePort() {
        return m_sourcePort;
    }

    TScenarioItemPort * getTargetPort() {
        return m_targetPort;
    }

    qreal getPreferredLineBreakCoord() const
    {
        return m_preferredLineBreakCoord;
    }

    void setPreferredLineBreakCoord(qreal value)
    {
        m_preferredLineBreakCoord = value;
    }

private:
    qreal m_preferredLineBreakCoord;

    TScenarioItemPort * m_sourcePort;
    TScenarioItemPort * m_targetPort;
};




#endif // TSCENARIOCONNECTION_H
