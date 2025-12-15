#include "tscenariographicalitemport.h"

#include "tscenariographicalconnection.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

TScenarioGraphicalItemPort::TScenarioGraphicalItemPort(TScenarioItemPort * scenarioItemPort, QGraphicsItem * parent)
    : QGraphicsPolygonItem(parent), m_scenarioItemPort(scenarioItemPort)
{
    if(!scenarioItemPort->getLabelText().isEmpty()) {
        m_labelTextItem = new QGraphicsSimpleTextItem(scenarioItemPort->getLabelText(), this);
        m_labelTextItem->setParentItem(this);
        m_labelTextItem->setPos(scenarioItemPort->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort ? 15 : -15 - m_labelTextItem->boundingRect().width(), -8);
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
    else if(scenarioItemPort->getType() == TScenarioItemPort::TItemPortType::TDataPort) {
        m_colorStrip->setPen(QPen(DATA_PORT_COLOR, 1));
        m_colorStrip->setBrush(QBrush(DATA_PORT_COLOR, Qt::SolidPattern));
    }
    else {
        m_colorStrip->setPen(QPen(CONN_PORT_COLOR, 1));
        m_colorStrip->setBrush(QBrush(CONN_PORT_COLOR, Qt::SolidPattern));
    }

    m_lastScenePos = scenePos();

    updateTooltip();
}

void TScenarioGraphicalItemPort::updateTooltip() {
    QString toolTipText;

    if(!m_scenarioItemPort->getLabelText().isEmpty()) {
        toolTipText.append("<b>" + m_scenarioItemPort->getLabelText() + "</b> " + m_scenarioItemPort->getDataTypeHint() + "<br>");
    }
    else if(!m_scenarioItemPort->getDataTypeHint().isEmpty()) {
        toolTipText.append(m_scenarioItemPort->getDataTypeHint() + "<br>");
    }

    toolTipText.append("<i><b>");
    toolTipText.append(m_scenarioItemPort->getName());
    toolTipText.append("</b> (");
    toolTipText.append(m_scenarioItemPort->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort ? "input " : "output ");

    if(m_scenarioItemPort->getType() == TScenarioItemPort::TItemPortType::TFlowPort) {
        toolTipText.append("flow ");
    }
    else if(m_scenarioItemPort->getType() == TScenarioItemPort::TItemPortType::TDataPort) {
        toolTipText.append("data ");
    }
    else {
        toolTipText.append("connection ");
    }

    toolTipText.append("port)</i>");

    if(!m_scenarioItemPort->getDescription().isEmpty()) {
        toolTipText.append("<br>" + m_scenarioItemPort->getDescription());
    }

    setToolTip(toolTipText);
}

TScenarioGraphicalItemPort::~TScenarioGraphicalItemPort() {
    // m_scenarioItemPort is deleted by its TScenarioItem
    delete m_labelTextItem;
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
    m_scenarioItemPort->clearConnectedPorts();
}

void TScenarioGraphicalItemPort::addGraphicalConnection(TScenarioGraphicalConnection * connection) {
    m_scenarioGraphicalConnections.append(connection);

    TScenarioGraphicalItemPort * otherItemPort = connection->otherItemPort(this);
    if(otherItemPort) {
        m_scenarioItemPort->addConnectedPort(otherItemPort->m_scenarioItemPort);
    }
}

bool TScenarioGraphicalItemPort::removeGraphicalConnection(TScenarioGraphicalConnection * connection) {
    TScenarioGraphicalItemPort * otherItemPort = connection->otherItemPort(this);
    if(otherItemPort) {
        m_scenarioItemPort->removeConnectedPort(otherItemPort->m_scenarioItemPort);
    }

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
