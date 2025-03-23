#include "tscenariographicalitemport.h"

#include "tscenariographicalconnection.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

TScenarioGraphicalItemPort::TScenarioGraphicalItemPort(TScenarioItemPort * scenarioItemPort, QGraphicsItem * parent)
    : QGraphicsPolygonItem(parent), m_scenarioItemPort(scenarioItemPort)
{
    QString toolTipText;

    if(!scenarioItemPort->getDisplayName().isEmpty()) {
        toolTipText.append("<b>" + scenarioItemPort->getDisplayName() + "</b><br>");
        m_titleText = new QGraphicsSimpleTextItem(scenarioItemPort->getDisplayName(), this);
        m_titleText->setParentItem(this);
        m_titleText->setPos(scenarioItemPort->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort ? 15 : -15 - m_titleText->boundingRect().width(), -8);
    }

    QPainterPath path;
    path.addRoundedRect(scenarioItemPort->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort ? -5 : -10, -7.5, 15, 15, 3, 3);
    setPolygon(path.toFillPolygon());
    setPen(QPen(QColor::fromString("#000"), 1));
    setBrush(QBrush(QColor::fromString("#fff"), Qt::SolidPattern));
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    m_colorStrip = new QGraphicsPolygonItem(this);
    m_colorStrip->setParentItem(this);
    m_colorStrip->setPolygon(QPolygonF(QRectF(scenarioItemPort->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort ? -5 : 2, -7.5, 3, 15)));


    if(scenarioItemPort->getType() == TScenarioItemPort::TItemPortType::TFlowPort) {
        m_colorStrip->setPen(QPen(FLOW_PORT_COLOR, 1));
        m_colorStrip->setBrush(QBrush(FLOW_PORT_COLOR, Qt::SolidPattern));
    }
    else {
        m_colorStrip->setPen(QPen(DATA_PORT_COLOR, 1));
        m_colorStrip->setBrush(QBrush(DATA_PORT_COLOR, Qt::SolidPattern));
    }

    toolTipText.append("<i><b>");
    toolTipText.append(scenarioItemPort->getName());
    toolTipText.append("</b> (");
    toolTipText.append(scenarioItemPort->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort ? "input " : "output ");
    toolTipText.append(scenarioItemPort->getType() == TScenarioItemPort::TItemPortType::TFlowPort ? "flow " : "data ");
    toolTipText.append("port)</i>");

    if(!scenarioItemPort->getDescription().isEmpty()) {
        toolTipText.append("<br>" + scenarioItemPort->getDescription());
    }

    setToolTip(toolTipText);

    m_lastScenePos = scenePos();
}

TScenarioGraphicalItemPort::~TScenarioGraphicalItemPort() {
    // m_scenarioItemPort is deleted by its TScenarioItem
}

TScenarioItemPort * TScenarioGraphicalItemPort::getScenarioItemPort() {
    return m_scenarioItemPort;
}

TScenarioItemPort::TItemPortType TScenarioGraphicalItemPort::portType() const {
    return m_scenarioItemPort->getType();
}

TScenarioItemPort::TItemPortDirection TScenarioGraphicalItemPort::portDirection() const {
    return m_scenarioItemPort->getDirection();
}

bool TScenarioGraphicalItemPort::hasGraphicalConnection() const {
    return !m_scenarioGraphicalConnections.empty();
}

void TScenarioGraphicalItemPort::removeGraphicalConnections() {
    m_scenarioGraphicalConnections.clear();
}

void TScenarioGraphicalItemPort::addGraphicalConnection(TScenarioGraphicalConnection * connection) {
    m_scenarioGraphicalConnections.append(connection);
}

bool TScenarioGraphicalItemPort::removeGraphicalConnection(TScenarioGraphicalConnection * connection) {
    return m_scenarioGraphicalConnections.removeAll(connection);
}

QList<TScenarioGraphicalConnection *> TScenarioGraphicalItemPort::getGraphicalConnections() {
    return m_scenarioGraphicalConnections;
}

QVariant TScenarioGraphicalItemPort::itemChange(GraphicsItemChange change, const QVariant &value) {
    if (change == QGraphicsItem::ItemScenePositionHasChanged){
        QPointF newScenePos = scenePos();

        QPointF deltaScenePos = QPointF(0, 0);
        if(m_scenarioItemPort->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort) {
            deltaScenePos = newScenePos - m_lastScenePos;
        }

        for(TScenarioGraphicalConnection * scenarioGraphicalConnection : m_scenarioGraphicalConnections) {
            scenarioGraphicalConnection->updatePosition(deltaScenePos);
        }

        m_lastScenePos = newScenePos;
    }

    return value;
}
