#ifndef TSCENARIOGRAPHICALFLOWSTARTITEM_H
#define TSCENARIOGRAPHICALFLOWSTARTITEM_H

#include <QGraphicsPixmapItem>
#include <QList>
#include <QPainter>
#include <QGuiApplication>
#include <QPalette>

#include "../tscenariographicalitem.h"

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
        ) : TScenarioGraphicalItem(scenarioItem, parent) { }

    void updateColors() override {
        TScenarioGraphicalItem::updateColors();
        setDefaultBrush(QBrush(QGuiApplication::palette().color(QPalette::WindowText), Qt::SolidPattern));
        updateTooltip();
    }

    QPixmap image() const override {
        QPixmap pixmap(250, 250);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setPen(QPen(QGuiApplication::palette().color(QPalette::WindowText), 8));
        painter.setBrush(QBrush(QGuiApplication::palette().color(QPalette::WindowText), Qt::SolidPattern));

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
