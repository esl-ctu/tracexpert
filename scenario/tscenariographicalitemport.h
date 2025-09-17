#ifndef TCONNECTOR_H
#define TCONNECTOR_H

#include <QGraphicsPixmapItem>
#include <QList>

#include "tscenarioitemport.h"

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

class TScenarioGraphicalConnection;

/*! \brief The TScenarioGraphicalItemPort class represents a graphical scenario port.
 *
 * The class represents a graphical scenario port based on a TScenarioItemPort.
 * It is a polygon item with a color strip and a title text.
 * It keeps a list of graphical connections and has methods for adding, removing and getting them.
 *
 */
class TScenarioGraphicalItemPort : public QGraphicsPolygonItem
{

public:
    TScenarioGraphicalItemPort(TScenarioItemPort * scenarioItemPort, QGraphicsItem * parent = nullptr);
    ~TScenarioGraphicalItemPort();

    TScenarioItemPort * getScenarioItemPort();

    enum { Type = UserType + 32 };
    int type() const override { return Type; }

    QList<TScenarioGraphicalConnection *> getGraphicalConnections();
    bool hasGraphicalConnection() const;
    void addGraphicalConnection(TScenarioGraphicalConnection * connection);
    bool removeGraphicalConnection(TScenarioGraphicalConnection * connection);
    void removeGraphicalConnections();

    TScenarioItemPort::TItemPortType portType() const;
    TScenarioItemPort::TItemPortDirection portDirection() const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    const QColor FLOW_PORT_COLOR = QColor::fromString("#89bac9");
    const QColor DATA_PORT_COLOR = QColor::fromString("#da9a85");
    const QColor CONN_PORT_COLOR = QColor::fromString("#9a85da");

    QPointF m_lastScenePos;

    QGraphicsPolygonItem * m_colorStrip;
    TScenarioItemPort * m_scenarioItemPort;

    QGraphicsSimpleTextItem * m_titleText = nullptr;

    QList<TScenarioGraphicalConnection *> m_scenarioGraphicalConnections;
};

#endif // TCONNECTOR_H
