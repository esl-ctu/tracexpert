#include "tscenariographicalitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QPainter>

#include "graphical_items/tscenariographicalembeddedsubtitleitem.h"
#include "../tdialog.h"
#include "tscenariographicalitemport.h"
#include "tscenariographicalconnection.h"
#include "tscenarioitem.h"
#include "graphical_items/tscenariographicalflowstartitem.h"
#include "graphical_items/tscenariographicalflowenditem.h"
#include "graphical_items/tscenariographicalflowmergeitem.h"
#include "graphical_items/tscenariographicalconditionitem.h"

TScenarioGraphicalItem * TScenarioGraphicalItem::createScenarioGraphicalItem(TScenarioItem * scenarioItem, QGraphicsItem * parent) {
    switch(scenarioItem->getType()) {
        case TScenarioItem::TItemAppearance::TFlowStart:
            return new TScenarioGraphicalFlowStartItem(scenarioItem, parent);
        case TScenarioItem::TItemAppearance::TFlowEnd:
            return new TScenarioGraphicalFlowEndItem(scenarioItem, parent);
        case TScenarioItem::TItemAppearance::TFlowMerge:
            return new TScenarioGraphicalFlowMergeItem(scenarioItem, parent);
        case TScenarioItem::TItemAppearance::TCondition:
            return new TScenarioGraphicalConditionItem(scenarioItem, parent);
        case TScenarioItem::TItemAppearance::TEmbeddedSubtitle:
            return new TScenarioGraphicalEmbeddedSubtitleItem(scenarioItem, parent);
        default:
            return new TScenarioGraphicalItem(scenarioItem, parent);
    }
}

TScenarioGraphicalItem::TScenarioGraphicalItem(TScenarioItem * scenarioItem, QGraphicsItem * parent)
    : QObject(), QGraphicsPathItem(parent), m_scenarioItem(scenarioItem), m_defaultBrush(QBrush(Qt::white, Qt::SolidPattern))
{
    setPos(scenarioItem->getPosition());

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setBrush(m_defaultBrush);

    if(scenarioItem->getType() == TScenarioItem::TItemAppearance::TDefault ||
        scenarioItem->getType() == TScenarioItem::TItemAppearance::TEmbeddedSubtitle)
    {
        m_titleText = new QGraphicsSimpleTextItem(this);
        m_titleText->setParentItem(this);

        QFont font = m_titleText->font();
        font.setBold(true);
        m_titleText->setFont(font);

        m_subtitleText = new QGraphicsSimpleTextItem(this);
        m_subtitleText->setParentItem(this);
    }

    if(!m_scenarioItem->getParams().isEmpty()) {
        m_editableIcon = new QGraphicsPixmapItem(QPixmap(":/icons/editable.png"));
        m_editableIcon->setParentItem(this);
        m_editableIcon->setOpacity(0.25);
    }

    connect(m_scenarioItem, &TScenarioItem::appearanceChanged, this, &TScenarioGraphicalItem::updateBlockAppearance);
    connect(m_scenarioItem, &TScenarioItem::stateChanged, this, &TScenarioGraphicalItem::updateTooltip);
    connect(m_scenarioItem, &TScenarioItem::portsChanged, this, [this]() { updatePorts(); updateBlockAppearance(); });

    updateTooltip();
    updatePorts();
}

TScenarioGraphicalItem::~TScenarioGraphicalItem() {
    delete m_scenarioItem;
    qDeleteAll(m_graphicalItemPorts);
}

TScenarioItem * TScenarioGraphicalItem::getScenarioItem() {
    return m_scenarioItem;
}

void TScenarioGraphicalItem::setScene(TScenarioScene * scene) {
    m_scene = scene;
}

void TScenarioGraphicalItem::updateTooltip() {
    QString toolTipText = "<b>" + m_scenarioItem->getName() + "</b><br><i>" + m_scenarioItem->getDescription() + "</i><br>";

    TScenarioItem::TState state = m_scenarioItem->getState();
    switch(state) {
        case TScenarioItem::TState::TError:
        case TScenarioItem::TState::TRuntimeError:
            setBrush(QBrush(QColor::fromRgb(255, 200, 200), Qt::SolidPattern));
            toolTipText.append("<span style=\"color:red\"><b><i>" + m_scenarioItem->getStateMessage() + "</i></b></span>");
            break;
        case TScenarioItem::TState::TWarning:
        case TScenarioItem::TState::TRuntimeWarning:
            setBrush(QBrush(QColor::fromRgb(255, 255, 200), Qt::SolidPattern));
            toolTipText.append("<span style=\"color:orange\"><b><i>" + m_scenarioItem->getStateMessage() + "</i></b></span>");
            break;
        case TScenarioItem::TState::TInfo:
        case TScenarioItem::TState::TRuntimeInfo:
        case TScenarioItem::TState::TBeingExecuted:
            setBrush(QBrush(QColor::fromRgb(225, 235, 255), Qt::SolidPattern));
            toolTipText.append("<span style=\"color:blue\"><b><i>" + m_scenarioItem->getStateMessage() + "</i></b></span>");
            break;
        default:
            setBrush(m_defaultBrush);
            break;
    }

    setToolTip(toolTipText);
}


QPixmap TScenarioGraphicalItem::image() const
{
    QPixmap pixmap(250, 250);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setPen(QPen(Qt::black, 8));

    const QString iconPath = m_scenarioItem->getIconResourcePath();
    if(iconPath.isEmpty()) {

        painter.drawRoundedRect(25, 50, 200, 150, 15, 15);
        painter.drawLine(25, 85, 225, 85);
    }
    else {
        QPixmap iconPixmap(iconPath);
        painter.drawPixmap(50, 50, 150, 150, iconPixmap);
    }

    return pixmap;
}

void TScenarioGraphicalItem::updateBlockAppearance() {

    int textWidth = 0;
    if(m_titleText) {
        QString elidedTitleText = QFontMetrics(m_titleText->font()).elidedText(m_scenarioItem->getTitle(), Qt::ElideRight, 220);
        m_titleText->setText(elidedTitleText);
        textWidth = m_titleText->boundingRect().width();
    }

    if(m_subtitleText) {
        int subtitleWidth = fmax(150 - 20, textWidth);
        QString elidedSubtitleText = QFontMetrics(m_subtitleText->font()).elidedText(m_scenarioItem->getSubtitle(), Qt::ElideRight, subtitleWidth);
        m_subtitleText->setText(elidedSubtitleText);
    }

    int width = fmax(150, textWidth + 20);
    int topOffset = 0;

    QPainterPath path;
    if(m_titleText && m_subtitleText) {
        if(!m_scenarioItem->getTitle().isEmpty()) {
            m_titleText->setPos(10, 5 + topOffset);
            path.moveTo(0,      24 + topOffset);
            path.lineTo(width,  24 + topOffset);
            path.moveTo(0,      26 + topOffset);
            path.lineTo(width,  26 + topOffset);
            topOffset += 26;
        }

        if(!m_scenarioItem->getSubtitle().isEmpty()) {
            m_subtitleText->setPos(10, 5 + topOffset);
            path.moveTo(0,      24 + topOffset);
            path.lineTo(width,  24 + topOffset);
            topOffset += 24;
        }
    }

    topOffset += 20;

    int leftConnectorCount = 0;
    int rightConnectorCount = 0;
    for(TScenarioGraphicalItemPort * itemPort : m_graphicalItemPorts) {
        if(itemPort->portDirection() == TScenarioItemPort::TItemPortDirection::TInputPort) {
            itemPort->setPos(QPointF(0, topOffset + VERTICAL_PORT_OFFSET*leftConnectorCount));
            leftConnectorCount++;
        }
        else {
            itemPort->setPos(QPointF(width, topOffset + VERTICAL_PORT_OFFSET*rightConnectorCount));
            rightConnectorCount++;
        }
    }

    topOffset += 20;

    // redraw the polygon itself
    int height = fmax(100, topOffset + VERTICAL_PORT_OFFSET*fmax(leftConnectorCount, rightConnectorCount));

    if(m_editableIcon) {
        m_editableIcon->setPos(width - 20, height - 20);
    }

    path.addRoundedRect(0, 0, width, height, 10, 10);
    setPath(path);
}

QList<TScenarioGraphicalItemPort *> & TScenarioGraphicalItem::getGraphicalItemPorts() {
    return m_graphicalItemPorts;
}

TScenarioGraphicalItemPort * TScenarioGraphicalItem::getGraphicalItemPortByName(const QString & name) {
    for(TScenarioGraphicalItemPort * p : m_graphicalItemPorts) {
        if (p->getScenarioItemPort()->getName() == name) {
            return p;
        }
    }

    return nullptr;
}

QVariant TScenarioGraphicalItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemChildAddedChange
        || change == QGraphicsItem::ItemSceneChange) {
        updateBlockAppearance();
    }
    else if (change == QGraphicsItem::ItemPositionHasChanged) {
        // snap items connected by flow into same y coordinate
        qreal minDist = SNAP_DISTANCE_THRESHOLD;
        for(TScenarioGraphicalItemPort * port : m_graphicalItemPorts) {
            for(TScenarioGraphicalConnection * connection : port->getGraphicalConnections()) {
                if(connection->isSelected()) {
                    continue;
                }

                qreal dist;
                if(port->portDirection() == TScenarioItemPort::TItemPortDirection::TInputPort) {
                    dist = port->scenePos().y() - connection->startItem()->scenePos().y();
                }
                else {
                    dist = port->scenePos().y() - connection->endItem()->scenePos().y();
                }

                if(abs(dist) < minDist) {
                    minDist = dist;
                }
            }
        }

        if(abs(minDist) < SNAP_DISTANCE_THRESHOLD) {
            setPos(pos().x(), pos().y() - minDist);
        }

        m_scenarioItem->setPosition(pos());
    }

    return value;
}

void TScenarioGraphicalItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(!m_scenarioItem->getParams().isEmpty()) {
        TScenarioConfigParamDialog * dialog = new TScenarioConfigParamDialog("OK", "Edit block parameters", m_scenarioItem);
        QSize originalSize = dialog->size();
        dialog->exec();

        if(dialog->size() != originalSize) {
            m_scenarioItem->setConfigWindowSize(dialog->size());
        }

        dialog->deleteLater();
    }    
}

void TScenarioGraphicalItem::updatePorts() {
    QList<TScenarioItemPort *> scenarioItemPorts = m_scenarioItem->getItemPorts();

    QList<TScenarioGraphicalItemPort *>::iterator it = m_graphicalItemPorts.begin();
    while (it != m_graphicalItemPorts.end()) {
        if(!scenarioItemPorts.removeOne((*it)->getScenarioItemPort())) {
            TScenarioGraphicalItemPort * itemPort = *it;
            it = m_graphicalItemPorts.erase(it);

            if(itemPort->hasGraphicalConnection()) {
                if(m_scene) {
                    for(TScenarioGraphicalConnection * graphicalConnection : itemPort->getGraphicalConnections()) {
                        m_scene->removeItem(graphicalConnection);
                    }
                }
                else {
                    qWarning("Failed to remove connection deleted by port removal.");
                }
            }

            delete itemPort;
        }
        else {
            ++it;
        }
    }

    for(TScenarioItemPort * itemPort : scenarioItemPorts) {
        m_graphicalItemPorts.append(new TScenarioGraphicalItemPort(itemPort, this));
    }

    std::sort(m_graphicalItemPorts.begin(), m_graphicalItemPorts.end(),
        [=] (TScenarioGraphicalItemPort * ip1, TScenarioGraphicalItemPort * ip2)->bool {
            int ip1Rank = (int)ip1->getScenarioItemPort()->getDirection() * 2 + (int)ip1->getScenarioItemPort()->getType();
            int ip2Rank = (int)ip2->getScenarioItemPort()->getDirection() * 2 + (int)ip2->getScenarioItemPort()->getType();
            return ip1Rank < ip2Rank;
        }
    );    
}

void TScenarioGraphicalItem::setDefaultBrush(QBrush brush) {
    m_defaultBrush = brush;
    setBrush(m_defaultBrush);
}



