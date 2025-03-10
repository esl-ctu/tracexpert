#ifndef TSCENARIOGRAPHICALFLOWMERGEITEM_H
#define TSCENARIOGRAPHICALFLOWMERGEITEM_H

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPainter>

#include "../tscenariographicalitem.h"

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

/*!
 * \brief The TScenarioGraphicalFlowMergeItem class represents a graphical representation of a flow merge item in a scenario.
 *
 * The class is a graphical representation of a flow merge item in a scenario.
 * It is drawn as a rectangle shape with two ports on its sides.
 *
 */
class TScenarioGraphicalFlowMergeItem : public TScenarioGraphicalItem
{

public:
    TScenarioGraphicalFlowMergeItem(
        TScenarioItem * scenarioItem,
        QGraphicsItem * parent = nullptr
        ) : TScenarioGraphicalItem(scenarioItem, parent) { }

    QPixmap image() const override { QPixmap pixmap(250, 250);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setPen(QPen(Qt::black, 8));

        painter.translate(125, 125);

        QPolygonF polygon;
        polygon << QPointF(-40, 100) << QPointF(40, 100)
                << QPointF(40, -100) << QPointF(-40, -100)
                << QPointF(-40, 100);
        painter.drawPolyline(polygon);

        return pixmap;
    }

protected:
    void updateBlockAppearance() override {
        int topOffset = 20;

        QPainterPath path;

        int leftConnectorCount = 0;
        int rightConnectorCount = 0;
        for(TScenarioGraphicalItemPort * itemPort : m_graphicalItemPorts) {
            if(itemPort->portDirection() == TScenarioItemPort::TItemPortDirection::TInputPort) {
                itemPort->setPos(QPointF(0, topOffset + VERTICAL_PORT_OFFSET * leftConnectorCount));
                leftConnectorCount++;
            }
            else {
                itemPort->setPos(QPointF(WIDTH, topOffset + VERTICAL_PORT_OFFSET * rightConnectorCount));
                rightConnectorCount++;
            }
        }

        int height = fmax(MIN_HEIGHT, topOffset + VERTICAL_PORT_OFFSET * fmax(leftConnectorCount, rightConnectorCount));

        if(m_editableIcon) {
            m_editableIcon->setPos(WIDTH - ICON_CORNER_OFFSET, height - ICON_CORNER_OFFSET);
        }

        path.addRoundedRect(0, 0, WIDTH, height, 10, 10);
        setPath(path);
    }

private:
    const int WIDTH = 50;
    const int MIN_HEIGHT = 60;
};

#endif // TSCENARIOGRAPHICALFLOWMERGEITEM_H
