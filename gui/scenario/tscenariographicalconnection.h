#ifndef TSCENARIOARROW_H
#define TSCENARIOARROW_H

#include <QGraphicsLineItem>

#include "tscenarioconnection.h"

class TScenarioGraphicalItemPort;

/*! \brief The TScenarioGraphicalConnection class represents a graphical scenario connection.
 *
 * The class represents a graphical scenario connection between two ports, based on a TScenarioConnection.
 * It is a "polyline" (a line made up of segments) with a color and an arrow head.
 *
 * It lets the user manipulate the position of the connection by moving its center segment.
 * The position is then remembered through the "preferred line break coordinate".
 *
 */
class TScenarioGraphicalConnection : public QGraphicsLineItem
{
public:
    enum { Type = UserType + 4 };

    TScenarioGraphicalConnection(TScenarioGraphicalItemPort * startItemPort,
                                 TScenarioGraphicalItemPort * endItemPort,
                                 TScenarioConnection * scenarioConnection = nullptr,
                                 QGraphicsItem * parent = nullptr);

    ~TScenarioGraphicalConnection();

    TScenarioConnection * getScenarioConnection();

    int type() const override { return Type; }
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    TScenarioGraphicalItemPort * startItem() const { return m_startItemPort; }
    TScenarioGraphicalItemPort * endItem() const { return m_endItemPort; }

    void setPreferredLineBreakCoord(qreal value);
    void updatePosition();

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr) override;

private:
    const QColor FLOW_CONNECTION_COLOR = QColor::fromString("#99cfe0");
    const QColor DATA_CONNECTION_COLOR = QColor::fromString("#da9a85");

    const int CLEARANCE = 20;       // distance from block to first "bend" of the connection
    const int CLEARANCE_STEP = 10;  // distance addition for each following port
    const int ARROW_SIZE = 10;

    TScenarioConnection * m_scenarioConnection;

    TScenarioGraphicalItemPort * m_startItemPort;
    TScenarioGraphicalItemPort * m_endItemPort;

    bool m_hasPreferredLineBreakCoord;
    qreal m_preferredLineBreakCoord;

    void findPortOrdersAndClearances();
    int m_startOrder;
    int m_endOrder;
    int m_startClearance;
    int m_endClearance;

    QPolygonF m_polyline;

    void updateOutline();
    QPainterPath m_outline;

    void updateArrowHead();
    QPolygonF m_arrowHead;
};


#endif // TSCENARIOARROW_H
