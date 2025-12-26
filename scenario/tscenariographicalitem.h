// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Contributors to this file:
// Adam Švehla (initial author)

#ifndef TSCENARIOBLOCK_H
#define TSCENARIOBLOCK_H

#include <QGraphicsPixmapItem>
#include <QList>

#include "tscenariographicalitemport.h"
#include "tscenarioscene.h"

QT_BEGIN_NAMESPACE
class QPixmap;
class QGraphicsSceneContextMenuEvent;
class QMenu;
class QPolygonF;
QT_END_NAMESPACE

/*!
 * \brief The TScenarioGraphicalItem class represents a graphical scenario block.
 *
 * The class represents a graphical scenario block based on a TScenarioItem.
 * It is a block with ports on either side, a title and subtitle text.
 * It keeps a list of graphical ports and has methods for adding, removing and getting them.
 *
 * If the underlying block is "configurable" (has parameters),
 * it allows the user to double-click on the block to edit its parameters.
 *
 */
class TScenarioGraphicalItem : public QObject, public QGraphicsPathItem
{
    Q_OBJECT

public:
    static TScenarioGraphicalItem * createScenarioGraphicalItem(TScenarioItem * scenarioItem, QGraphicsItem * parent = nullptr);
    ~TScenarioGraphicalItem();

    TScenarioItem * getScenarioItem();
    void setScene(TScenarioScene * scene);

    enum { Type = UserType + 16 };
    int type() const override { return Type; }

    virtual QPixmap image() const;

    TScenarioGraphicalItemPort * getGraphicalItemPortByName(const QString & name);
    QList<TScenarioGraphicalItemPort *> & getGraphicalItemPorts();

protected:
    const int ICON_CORNER_OFFSET = 20;
    const int VERTICAL_PORT_OFFSET = 20;
    const int SNAP_DISTANCE_THRESHOLD = 20;

    TScenarioGraphicalItem(TScenarioItem * scenarioItem, QGraphicsItem * parent = nullptr);

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;

    virtual void updateBlockAppearance();

    QList<TScenarioGraphicalItemPort *> m_graphicalItemPorts;

    void setDefaultBrush(QBrush brush);

    QGraphicsPixmapItem * m_editableIcon = nullptr;
    QGraphicsSimpleTextItem * m_titleText = nullptr;
    QGraphicsSimpleTextItem * m_subtitleText = nullptr;

protected:
    const int WIDTH = 50;
    const int MIN_HEIGHT = 60;

    void updateTooltip();
    void updatePorts();

    TScenarioItem * m_scenarioItem;
    TScenarioScene * m_scene;

    QBrush m_defaultBrush;   
};

#endif // TSCENARIOBLOCK_H
