#include "tscenariographicalconnection.h"
#include "qgraphicsscene.h"
#include "qgraphicssceneevent.h"
#include "tscenarioitem.h"
#include "tscenariographicalitemport.h"

#include <QPainter>
#include <QPen>
#include <QtMath>

TScenarioGraphicalConnection::TScenarioGraphicalConnection(TScenarioGraphicalItemPort * startItemPort,
                                                           TScenarioGraphicalItemPort * endItemPort,
                                                           TScenarioConnection * scenarioConnection,
                                                           QGraphicsItem * parent
                                                           ) :
    QGraphicsLineItem(parent),
    m_startItemPort(startItemPort),
    m_endItemPort(endItemPort),
    m_startClearance(CLEARANCE),
    m_endClearance(CLEARANCE)
{
    m_scenarioConnection = scenarioConnection;

    if(!m_scenarioConnection) {
        m_scenarioConnection = new TScenarioConnection(startItemPort->getScenarioItemPort(), endItemPort->getScenarioItemPort());
    }

    m_preferredLineBreakCoord = m_scenarioConnection->getPreferredLineBreakCoord();

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    findPortOrdersAndClearances();
}

TScenarioGraphicalConnection::~TScenarioGraphicalConnection() {
    delete m_scenarioConnection;
}

TScenarioConnection * TScenarioGraphicalConnection::getScenarioConnection() {
    return m_scenarioConnection;
}

QRectF TScenarioGraphicalConnection::boundingRect() const {
    qreal extra = pen().width() + 50;
    return m_outline.boundingRect().normalized().adjusted(-extra, -extra, extra, extra);
}

QPainterPath TScenarioGraphicalConnection::shape() const {
    QPainterPath path;
    path.addPath(m_outline);
    path.addPolygon(m_arrowHead);
    return path;
}

TScenarioGraphicalItemPort * TScenarioGraphicalConnection::otherItemPort(TScenarioGraphicalItemPort * itemPort) const {
    if(itemPort == m_startItemPort) {
        return m_endItemPort;
    }
    else if(itemPort == m_endItemPort) {
        return m_startItemPort;
    }
    else {
        return nullptr;
    }
}

void TScenarioGraphicalConnection::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if(line().p1().x() > line().p2().x()) {
        setPreferredLineBreakYCoord(event->scenePos().y());
    }
    else {
        setPreferredLineBreakXCoord(event->scenePos().x());
    }

    updatePosition();
}

void TScenarioGraphicalConnection::setPreferredLineBreakXCoord(qreal value) {
    m_preferredLineBreakCoord = QPointF(value, 0);
    m_scenarioConnection->setPreferredLineBreakCoord(m_preferredLineBreakCoord);
}

void TScenarioGraphicalConnection::setPreferredLineBreakYCoord(qreal value) {
    m_preferredLineBreakCoord = QPointF(0, value);
    m_scenarioConnection->setPreferredLineBreakCoord(m_preferredLineBreakCoord);
}

void TScenarioGraphicalConnection::updatePosition(QPointF delta) {
    QLineF directLine(m_startItemPort->scenePos(), m_endItemPort->scenePos());
    setLine(directLine);

    if(m_preferredLineBreakCoord.y() > 0) {
        setPreferredLineBreakYCoord(m_preferredLineBreakCoord.y() + delta.y());
    }

    if(m_preferredLineBreakCoord.x() > 0) {
        setPreferredLineBreakXCoord(m_preferredLineBreakCoord.x() + delta.x());
    }


    m_polyline.clear();
    qreal lineBreakCoord;

    if(line().p1().y() > line().p2().y()) {
        m_startClearance = CLEARANCE + m_startOrder * CLEARANCE_STEP;
        m_endClearance = CLEARANCE + m_endOrder * CLEARANCE_STEP;
    }
    else {
        m_startClearance = CLEARANCE + (m_startBlockConnectorCount - m_startOrder) * CLEARANCE_STEP;
        m_endClearance = CLEARANCE + (m_endBlockConnectorCount - m_endOrder) * CLEARANCE_STEP;
    }

    if(line().p1().x() > line().p2().x()) {
        QPointF p1(line().p1().x(), line().p1().y());
        QPointF p2(line().p2().x(), line().p2().y());

        lineBreakCoord = (line().p1().y() + line().p2().y())/2;
        if(line().p1().x() + m_startClearance + m_endClearance > line().p2().x()) {
            p1.setX(line().p1().x() + m_startClearance);
            p2.setX(line().p2().x() - m_endClearance);
        }

        if(m_preferredLineBreakCoord.y() > 0) {
            lineBreakCoord = m_preferredLineBreakCoord.y();
        }
        else {
            setPreferredLineBreakYCoord(lineBreakCoord);
        }

        m_polyline.append(line().p1());
        m_polyline.append(p1);
        m_polyline.append(QPointF(p1.x(), lineBreakCoord));
        m_polyline.append(QPointF(p2.x(), lineBreakCoord));
        m_polyline.append(p2);
        m_polyline.append(line().p2());
    }
    else {
        lineBreakCoord = (line().p1().x() + line().p2().x())/2;

        if(lineBreakCoord + m_startClearance + CLEARANCE < line().p2().x()) {
            lineBreakCoord += m_startClearance;
        }

        if(m_preferredLineBreakCoord.x() > 0) {
            if(m_preferredLineBreakCoord.x() >= line().p1().x() + m_startClearance
                && m_preferredLineBreakCoord.x() <= line().p2().x() - m_endClearance)
            {
                lineBreakCoord = m_preferredLineBreakCoord.x();
            }
            else {
                if(m_preferredLineBreakCoord.x() < line().p1().x() + m_startClearance) {
                    lineBreakCoord = line().p1().x() + m_startClearance;
                } else if(m_preferredLineBreakCoord.x() > line().p2().x() - m_endClearance) {
                    lineBreakCoord = line().p2().x() - m_endClearance;
                }
            }
        }
        else {
            setPreferredLineBreakXCoord(lineBreakCoord);
        }

        m_polyline.append(line().p1());
        m_polyline.append(QPointF(lineBreakCoord, line().p1().y()));
        m_polyline.append(QPointF(lineBreakCoord, line().p2().y()));
        m_polyline.append(line().p2());
    }

    updateArrowHead();
    updateOutline();

    update();
}

void TScenarioGraphicalConnection::updateArrowHead() {
    m_arrowHead.clear();
    QPointF arrowP1 = line().p2() + QPointF(-0.866 * ARROW_SIZE, -0.5 * ARROW_SIZE);
    QPointF arrowP2 = line().p2() + QPointF(-0.866 * ARROW_SIZE, 0.5 * ARROW_SIZE);
    m_arrowHead << line().p2() << arrowP1 << arrowP2;
}

void TScenarioGraphicalConnection::updateOutline() {
    m_outline.clear();
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    for(int i = 0; i < m_polyline.size()-1; i++) {
        QPointF p1 = m_polyline[i].x() < m_polyline[i+1].x() ? m_polyline[i] : m_polyline[i+1];
        QPointF p2 = m_polyline[i].x() < m_polyline[i+1].x() ? m_polyline[i+1] : m_polyline[i];
        path.addRect(QRectF(QPointF(p1.x() - 5, p1.y() - 5), QPointF(p2.x() + 5, p2.y() + 5)));
    }
    m_outline = path.simplified();
}

void TScenarioGraphicalConnection::findPortOrdersAndClearances() {
    TScenarioItem * startParent = m_startItemPort->getScenarioItemPort()->getParentItem();
    m_startOrder = startParent->getItemPortSideOrderByName(m_startItemPort->getScenarioItemPort()->getName());
    m_startBlockConnectorCount = startParent->getItemPortCountByDirection(TScenarioItemPort::TItemPortDirection::TOutputPort);
    m_startClearance = CLEARANCE + m_startOrder * CLEARANCE_STEP;

    TScenarioItem * endParent = m_endItemPort->getScenarioItemPort()->getParentItem();
    m_endOrder = endParent->getItemPortSideOrderByName(m_endItemPort->getScenarioItemPort()->getName());
    m_endBlockConnectorCount = startParent->getItemPortCountByDirection(TScenarioItemPort::TItemPortDirection::TInputPort);
    m_endClearance = CLEARANCE + m_endOrder * CLEARANCE_STEP;
}

void TScenarioGraphicalConnection::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_startItemPort->collidesWithItem(m_endItemPort))
        return;

    QColor color;
    // draw different line style based on port type
    if(m_startItemPort->getScenarioItemPort()->getType() == TScenarioItemPort::TItemPortType::TFlowPort) {
        // draw simple line for flow lines
        color = FLOW_LINE_COLOR;
        painter->setPen(QPen(color, 2));
        painter->drawPolyline(m_polyline);
    }
    else if(m_startItemPort->getScenarioItemPort()->getType() == TScenarioItemPort::TItemPortType::TDataPort) {
        // draw double line for data lines
        color = DATA_LINE_COLOR;
        painter->setPen(QPen(color, 5));
        painter->drawPolyline(m_polyline);
        painter->setPen(QPen(Qt::white, 1));
        painter->drawPolyline(m_polyline);
    }
    else {
        // draw simple line for connection lines
        color = CONN_LINE_COLOR;
        painter->setPen(QPen(color, 2));
        painter->drawPolyline(m_polyline);
    }

    // draw arrow head
    painter->setPen(QPen(color, 2));
    painter->setBrush(color);
    painter->drawPolygon(m_arrowHead);

    // draw dashed outline if selected
    if (isSelected()) {
        painter->setPen(QPen(color, 1, Qt::DashLine));
        painter->setBrush(QColor::fromRgb(0,0,0,0));
        painter->drawPath(m_outline);
    }
}
