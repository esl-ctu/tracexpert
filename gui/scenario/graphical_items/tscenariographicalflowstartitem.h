#ifndef TSCENARIOGRAPHICALFLOWSTARTITEM_H
#define TSCENARIOGRAPHICALFLOWSTARTITEM_H

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
 * \brief The TScenarioGraphicalFlowStartItem class represents a graphical representation of a flow start item in a scenario.
 *
 * The class is a graphical representation of a flow start item in a scenario.
 * It is drawn as an ellipse shape with one output port on its side.
 *
 */
class TScenarioGraphicalFlowStartItem : public TScenarioGraphicalItem
{

public:
    TScenarioGraphicalFlowStartItem(
        TScenarioItem * scenarioItem,
        QGraphicsItem * parent = nullptr
        ) : TScenarioGraphicalItem(scenarioItem, parent) {
        setDefaultBrush(QBrush(Qt::black, Qt::SolidPattern));
    }

    QPixmap image() const override {
        QPixmap pixmap(250, 250);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setPen(QPen(Qt::black, 8));
        painter.setBrush(QBrush(Qt::black, Qt::SolidPattern));

        QPainterPath path;
        path.addEllipse(50, 50, 150, 150);
        painter.drawPolygon(path.toFillPolygon());

        return pixmap;
    }

protected:
    void updateBlockAppearance() override {
        if(m_graphicalItemPorts.count() > 0) {
            m_graphicalItemPorts[0]->setPos(QPointF(30, 15));
        }

        QPainterPath path;
        path.addEllipse(0, 0, 30, 30);
        setPath(path);
    }
};

#endif // TSCENARIOGRAPHICALFLOWSTARTITEM_H
