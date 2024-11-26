#include "tscenariographicalconnection.h"
#include "qgraphicsscene.h"
#include "qgraphicssceneevent.h"
#include "scenario/tscenarioitem.h"
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
    m_hasPreferredLineBreakCoord(false),
    m_startClearance(CLEARANCE),
    m_endClearance(CLEARANCE)
{
    m_scenarioConnection = scenarioConnection;

    if(!m_scenarioConnection) {
        m_scenarioConnection = new TScenarioConnection(startItemPort->getScenarioItemPort(), endItemPort->getScenarioItemPort());
    }

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

void TScenarioGraphicalConnection::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    qreal preferredLineBreakCoord;
    if(line().p1().x() + m_startClearance + m_endClearance > line().p2().x()) {
        preferredLineBreakCoord = event->scenePos().y();
    }
    else {
        preferredLineBreakCoord = event->scenePos().x();
    }

    setPreferredLineBreakCoord(preferredLineBreakCoord);

    updatePosition();
}

void TScenarioGraphicalConnection::setPreferredLineBreakCoord(qreal value) {
    m_preferredLineBreakCoord = value;
    m_scenarioConnection->setPreferredLineBreakCoord(m_preferredLineBreakCoord);
    m_hasPreferredLineBreakCoord = true;
}

void TScenarioGraphicalConnection::updatePosition() {
    QLineF directLine(m_startItemPort->scenePos(), m_endItemPort->scenePos());
    setLine(directLine);

    m_polyline.clear();
    qreal lineBreakCoord;

    if(line().p1().x() + m_startClearance + m_endClearance > line().p2().x()) {
        lineBreakCoord = (line().p1().y() + line().p2().y())/2 + m_startOrder * (CLEARANCE + CLEARANCE_STEP);
        if(m_hasPreferredLineBreakCoord) {
            lineBreakCoord = m_preferredLineBreakCoord;
        }
        else {
            setPreferredLineBreakCoord(lineBreakCoord);
        }

        QPointF p1(line().p1().x() + m_startClearance, line().p1().y());
        QPointF p2(line().p2().x() - m_endClearance, line().p2().y());
        m_polyline.append(line().p1());
        m_polyline.append(p1);
        m_polyline.append(QPointF(p1.x(), lineBreakCoord));
        m_polyline.append(QPointF(p2.x(), lineBreakCoord));
        m_polyline.append(p2);
        m_polyline.append(line().p2());
    }
    else {
        lineBreakCoord = (line().p1().x() + line().p2().x())/2 + m_startOrder * CLEARANCE_STEP;
        if(m_hasPreferredLineBreakCoord) {
            if(m_preferredLineBreakCoord >= line().p1().x() + m_startClearance
                && m_preferredLineBreakCoord <= line().p2().x() - m_endClearance)
            {
                lineBreakCoord = m_preferredLineBreakCoord;
            }
            else {

                if(m_preferredLineBreakCoord < line().p1().x() + m_startClearance) {
                    lineBreakCoord = line().p1().x() + m_startClearance;
                } else if(m_preferredLineBreakCoord > line().p2().x() - m_endClearance) {
                    lineBreakCoord = line().p2().x() - m_endClearance;
                }
            }
        }
        else {
            setPreferredLineBreakCoord(lineBreakCoord);
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
    m_startClearance = CLEARANCE + m_startOrder * CLEARANCE_STEP;

    TScenarioItem * endParent = m_endItemPort->getScenarioItemPort()->getParentItem();
    m_endOrder = endParent->getItemPortSideOrderByName(m_endItemPort->getScenarioItemPort()->getName());
    m_endClearance = CLEARANCE + m_endOrder * CLEARANCE_STEP;
}

void TScenarioGraphicalConnection::paint(QPainter * painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_startItemPort->collidesWithItem(m_endItemPort))
        return;

    QColor color;
    // draw different line style based on port type
    if(m_startItemPort->getScenarioItemPort()->getType() == TScenarioItemPort::TItemPortType::TFlowPort) {
        // draw simple line for flow lines
        color = FLOW_CONNECTION_COLOR;
        painter->setPen(QPen(color, 2));
        painter->drawPolyline(m_polyline);
    }
    else {
        // draw double line for data lines
        color = DATA_CONNECTION_COLOR;
        painter->setPen(QPen(color, 5));
        painter->drawPolyline(m_polyline);
        painter->setPen(QPen(Qt::white, 1));
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
