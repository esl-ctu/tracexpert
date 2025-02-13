#ifndef TSCENARIOGRAPHICALFLOWENDITEM_H
#define TSCENARIOGRAPHICALFLOWENDITEM_H

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
 * @brief The TScenarioGraphicalFlowEndItem class represents a graphical representation of a flow end item in a scenario.
 *
 * The class is a graphical representation of a flow end item in a scenario.
 * It is drawn as an ellipse shape with one input port on its side.
 *
 */
class TScenarioGraphicalFlowEndItem : public TScenarioGraphicalItem
{

public:
    TScenarioGraphicalFlowEndItem(
        TScenarioItem * scenarioItem,
        QGraphicsItem * parent = nullptr
        ) : TScenarioGraphicalItem(scenarioItem, parent) { }

    QPixmap image() const override {
        QPixmap pixmap(250, 250);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setPen(QPen(Qt::black, 8));

        QPainterPath path;
        path.addEllipse(50, 50, 150, 150);
        painter.drawPolygon(path.toFillPolygon());

        return pixmap;
    }

protected:
    void updateBlockAppearance() override {
        if(m_graphicalItemPorts.count() > 0) {
            m_graphicalItemPorts[0]->setPos(QPointF(0, 15));
        }

        QPainterPath path;
        path.addEllipse(0, 0, 30, 30);
        setPath(path);
    }
};

#endif // TSCENARIOGRAPHICALFLOWENDITEM_H
