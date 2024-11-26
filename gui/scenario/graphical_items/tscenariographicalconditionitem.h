#ifndef TSCENARIOGRAPHICALCONDITIONITEM_H
#define TSCENARIOGRAPHICALCONDITIONITEM_H

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPainter>

#include "scenario/tscenariographicalitem.h"

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

/*!
 * \brief The TScenarioGraphicalConditionItem class represents a graphical representation of a condition item in a scenario.
 *
 * The class is a graphical representation of a condition item in a scenario.
 * It is drawn as a diamond shape with four ports on its sides.
 *
 */
class TScenarioGraphicalConditionItem : public TScenarioGraphicalItem
{

public:
    TScenarioGraphicalConditionItem(
        TScenarioItem * scenarioItem,
        QGraphicsItem * parent = nullptr
        ) : TScenarioGraphicalItem(scenarioItem, parent) { }

    QPixmap image() const override {
        QPixmap pixmap(250, 250);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setPen(QPen(Qt::black, 8));
        painter.translate(125, 125);

        QPolygonF polygon;
        polygon << QPointF(-100, 0) << QPointF(0, 100)
                << QPointF(100, 0) << QPointF(0, -100)
                << QPointF(-100, 0);
        painter.drawPolyline(polygon);

        return pixmap;
    }

protected:
    void updateBlockAppearance() override {
        if(m_graphicalItemPorts.count() > 3) {
            m_graphicalItemPorts[0]->setPos(QPointF(-30, -10));
            m_graphicalItemPorts[1]->setPos(QPointF(-30, 10));
            m_graphicalItemPorts[2]->setPos(QPointF(30, 0));
            m_graphicalItemPorts[3]->setPos(QPointF(0, 30));
        }

        QPolygonF polygon;
        polygon << QPointF(-30, 0) << QPointF(0, 30)
                << QPointF(30, 0) << QPointF(0, -30)
                << QPointF(-30, 0);

        QPainterPath path;
        path.addPolygon(polygon);
        setPath(path);
    }
};

#endif // TSCENARIOGRAPHICALCONDITIONITEM_H
