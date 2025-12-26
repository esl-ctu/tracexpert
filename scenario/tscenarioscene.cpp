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
// Petr Socha

#include "tscenarioscene.h"
#include "tscenariographicalitemport.h"
#include "tscenariographicalconnection.h"
#include "tscenariographicalitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

TScenarioScene::TScenarioScene(TProjectModel * projectModel, QObject * parent) :
    QGraphicsScene(parent),
    m_pointerTool(TScenarioPointerTool::MousePointer),
    m_scenario(nullptr),
    m_projectModel(projectModel),
    m_tmpLine(nullptr),
    m_tmpLineStartPort(nullptr),
    m_tmpLineEndPort(nullptr)
{

}

TScenarioScene::~TScenarioScene() {
    for(auto item : items()) {
        // remove only "block" items, connections will get removed automatically
        if(item->type() == TScenarioGraphicalItem::Type) {
            removeItem(item);
        }
    }

    delete m_scenario;
}

TScenario * TScenarioScene::copyScenario() {
    return new TScenario(*m_scenario);
}

TScenario * TScenarioScene::scenario() {
    return m_scenario;
}

void TScenarioScene::loadScenario(const TScenario * scenario) {
    delete m_scenario;

    m_scenario = new TScenario(*scenario);

    this->clear();

    QHash<TScenarioItemPort *, TScenarioGraphicalItemPort *> itemPortMap;

    for(TScenarioItem * item : m_scenario->getItems()) {
        item->setProjectModel(m_projectModel);
        item->updateParams(false);
        item->resetState(true);

        TScenarioGraphicalItem * graphicalItem = TScenarioGraphicalItem::createScenarioGraphicalItem(item);
        graphicalItem->setZValue(1);
        graphicalItem->setScene(this);

        for(TScenarioGraphicalItemPort * itemPort : graphicalItem->getGraphicalItemPorts()) {
            itemPortMap.insert(itemPort->getScenarioItemPort(), itemPort);
        }

        addItem(graphicalItem);
    }

    for(TScenarioConnection * connection : m_scenario->getConnections()) {
        TScenarioGraphicalItemPort * sourcePort = itemPortMap.value(connection->getSourcePort());
        TScenarioGraphicalItemPort * targetPort = itemPortMap.value(connection->getTargetPort());
        TScenarioGraphicalConnection * graphicalConnection = new TScenarioGraphicalConnection(sourcePort, targetPort, connection);
        sourcePort->addGraphicalConnection(graphicalConnection);
        targetPort->addGraphicalConnection(graphicalConnection);
        graphicalConnection->setZValue(0);
        addItem(graphicalConnection);

        graphicalConnection->updatePosition();
    }
}

void TScenarioScene::setPointerTool(TScenarioPointerTool pointerTool)
{
    cleanupAfterCurrentTool();
    m_pointerTool = pointerTool;    
    emit pointerToolChanged(pointerTool);
}

TScenarioScene::TScenarioPointerTool TScenarioScene::pointerTool() {
    return m_pointerTool;
}

void TScenarioScene::removeItem(QGraphicsItem * item) {
    bool ok;

    if(item->type() == TScenarioGraphicalItem::Type) {
        TScenarioGraphicalItem * graphicalItem = (TScenarioGraphicalItem *)item;

        for(TScenarioGraphicalItemPort * itemPort : graphicalItem->getGraphicalItemPorts()) {
            for(TScenarioGraphicalConnection * graphicalConnection : itemPort->getGraphicalConnections()) {
                removeItem(graphicalConnection);
            }
        }


        if(m_scenario->hasItem(graphicalItem->getScenarioItem())) {
            m_scenario->removeItem(graphicalItem->getScenarioItem(), &ok);

            if(!ok) {
                qWarning("Failed to remove item from scenario.");
            }
        }
    }
    else if(item->type() == TScenarioGraphicalConnection::Type) {
        TScenarioGraphicalConnection * graphicalConnection = (TScenarioGraphicalConnection *)item;
        graphicalConnection->startItemPort()->removeGraphicalConnection(graphicalConnection);
        graphicalConnection->endItemPort()->removeGraphicalConnection(graphicalConnection);

        m_scenario->removeConnection(graphicalConnection->getScenarioConnection(), &ok);

        if(!ok) {
            qWarning("Failed to remove connection from scenario.");
        }
    }

    QGraphicsScene::removeItem(item);
}

void TScenarioScene::removeSelectedItems() {
    QList<QGraphicsItem *> selectedItems = this->selectedItems();

    if(selectedItems.size() == 0)
        return;

    const QString message =
        (selectedItems.size() == 1)
            ? tr("Are you sure you want to remove the selected item?")
            : tr("Are you sure you want to remove the %1 selected items?")
                  .arg(selectedItems.size());

    QMessageBox::StandardButton reply =
        QMessageBox::question(
            nullptr, // or your parent widget if you have one
            tr("Confirm removal"),
            message,
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
            );

    if (reply != QMessageBox::Yes)
        return;

    for (QGraphicsItem * item : selectedItems) {
        removeItem(item);
    }
}

void TScenarioScene::bringSelectedToFront() {
    if (this->selectedItems().isEmpty())
        return;

    const QList<QGraphicsItem *> selectedItems = this->selectedItems();
    QGraphicsItem * selectedItem = selectedItems.first();

    if(selectedItem->type() == TScenarioGraphicalItem::Type) {
        const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

        qreal zValue = 0;
        for (const QGraphicsItem *item : overlapItems) {
            if (item->zValue() >= zValue && item->type() == TScenarioGraphicalItem::Type)
                zValue = item->zValue() + 0.1;
        }
        selectedItem->setZValue(zValue);
    }
}

void TScenarioScene::sendSelectedToBack() {
    if (this->selectedItems().isEmpty())
        return;

    const QList<QGraphicsItem *> selectedItems = this->selectedItems();
    QGraphicsItem * selectedItem = selectedItems.first();

    if(selectedItem->type() == TScenarioGraphicalItem::Type) {
        const QList<QGraphicsItem *> overlapItems = selectedItem->collidingItems();

        qreal zValue = 0;
        for (const QGraphicsItem *item : overlapItems) {
            if (item->zValue() <= zValue && item->type() == TScenarioGraphicalItem::Type)
                zValue = item->zValue() - 0.1;
        }
        selectedItem->setZValue(zValue);
    }
}

void TScenarioScene::setInsertItemMode(TScenarioGraphicalItem * insertedBlockInstance) {
    setPointerTool(TScenarioPointerTool::InsertItem);

    m_insertedItemInstance = insertedBlockInstance;
    m_insertedItemInstance->setOpacity(0); // hide before cursor first over scene
    addItem(m_insertedItemInstance);

    emit itemInsertStarted(m_insertedItemInstance->getScenarioItem()->itemClass());
}

void TScenarioScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) {
    if(mouseEvent->button() == Qt::LeftButton) {
        switch (m_pointerTool) {
            case TScenarioPointerTool::InsertItem:
                bool ok;
                m_scenario->addItem(m_insertedItemInstance->getScenarioItem(), &ok);

                if(ok) {
                    m_insertedItemInstance->getScenarioItem()->setProjectModel(m_projectModel);
                    m_insertedItemInstance->getScenarioItem()->updateParams(false);
                    m_insertedItemInstance->setScene(this);
                    m_insertedItemInstance->setOpacity(1.0);
                    m_insertedItemInstance->setZValue(1);

                    m_pointerTool = TScenarioPointerTool::MousePointer;
                    emit itemInserted(m_insertedItemInstance->getScenarioItem()->itemClass());

                    // do not delete item, just lose the reference
                    // (it is now owned by the scene)
                    m_insertedItemInstance = nullptr;

                    qDebug("Item added to scenario.");
                    break;
                }

                qDebug("Failed to add item to scenario.");
                break;
            case TScenarioPointerTool::InsertLine: {
                    // cancel current line, if it exists
                    cleanupAfterCurrentTool();
                    // create a new line
                    m_tmpLineStartPort = findLineStartPort(mouseEvent->scenePos());

                    QPointF lineStartPoint = mouseEvent->scenePos();
                    if(m_tmpLineStartPort) {
                        lineStartPoint = m_tmpLineStartPort->scenePos();
                    }

                    m_tmpLine = new QGraphicsLineItem(QLineF(lineStartPoint, lineStartPoint));
                    m_tmpLine->setPen(QPen(Qt::black, 2));
                    m_tmpLine->setZValue(2);
                    addItem(m_tmpLine);
                    break;
                }
            case TScenarioPointerTool::MouseDrag:
                break;
            default:
                QGraphicsScene::mousePressEvent(mouseEvent);
                break;
        }
    }
    else if(mouseEvent->button() == Qt::RightButton) {
        setPointerTool(TScenarioPointerTool::MousePointer);
    }
    else {
        QGraphicsScene::mousePressEvent(mouseEvent);
    }
}

void TScenarioScene::cleanupAfterCurrentTool() {
    switch (m_pointerTool) {
        case TScenarioPointerTool::InsertItem:
            if(m_insertedItemInstance) {
                emit itemInsertCancelled(m_insertedItemInstance->getScenarioItem()->itemClass());
                removeItem(m_insertedItemInstance);
                delete m_insertedItemInstance;
                m_insertedItemInstance = nullptr;
            }
            break;
        case TScenarioPointerTool::InsertLine:
            if (m_tmpLine) {
                removeItem(m_tmpLine);
                delete m_tmpLine;
                m_tmpLine = nullptr;
            }
            break;
        default:
            break;
    }
}

void TScenarioScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent) {
    if(m_pointerTool == TScenarioPointerTool::InsertItem) {
        if(m_insertedItemInstance) {
            m_insertedItemInstance->setOpacity(0.75);
            m_insertedItemInstance->setPos(mouseEvent->scenePos());
        }
    }
    else if(m_pointerTool == TScenarioPointerTool::InsertLine && m_tmpLine != nullptr) {
        m_tmpLineEndPort = findLineEndPort(mouseEvent->scenePos());
        QPointF lineEndPoint = m_tmpLineEndPort == nullptr ? mouseEvent->scenePos() : m_tmpLineEndPort->scenePos();
        m_tmpLine->setLine(QLineF(m_tmpLine->line().p1(), lineEndPoint));
        m_tmpLine->setPen(evaluateLinePen());
    }
    else {
        QGraphicsScene::mouseMoveEvent(mouseEvent);
    }
}

void TScenarioScene::mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent) {
    if (m_tmpLine != nullptr && m_pointerTool == TScenarioPointerTool::InsertLine) {
        removeItem(m_tmpLine);
        delete m_tmpLine;
        m_tmpLine = nullptr;

        if (isLineValidArrow()) {
            TScenarioGraphicalItemPort * startConnector = (TScenarioGraphicalItemPort *)m_tmpLineStartPort;
            TScenarioGraphicalItemPort * endConnector = (TScenarioGraphicalItemPort *)m_tmpLineEndPort;
            TScenarioGraphicalConnection * scenarioGraphicalConnection = new TScenarioGraphicalConnection(startConnector, endConnector);

            bool ok;
            m_scenario->addConnection(scenarioGraphicalConnection->getScenarioConnection(), &ok);

            if(ok) {
                qDebug("Connection added to scenario.");
                startConnector->addGraphicalConnection(scenarioGraphicalConnection);
                endConnector->addGraphicalConnection(scenarioGraphicalConnection);

                scenarioGraphicalConnection->setZValue(0);

                addItem(scenarioGraphicalConnection);
                scenarioGraphicalConnection->updatePosition();
            }
            else {
                qDebug("Failed to add connection to scenario.");
            }

            m_tmpLineStartPort = nullptr;
            m_tmpLineEndPort = nullptr;
        }
    }

    QGraphicsScene::mouseReleaseEvent(mouseEvent);
}

TScenarioGraphicalItemPort * TScenarioScene::findLineStartPort(const QPointF & startPoint) {
    QList<QGraphicsItem *> startItems = items(startPoint);

    while(startItems.count() > 0) {
        if(startItems.first() == m_tmpLine ||
            (startItems.first()->type() != TScenarioGraphicalItem::Type && startItems.first()->type() != TScenarioGraphicalItemPort::Type)) {
            startItems.removeFirst();
            continue;
        }

        break;
    }

    if(startItems.count() > 0) {
        switch(startItems.first()->type()) {
            case TScenarioGraphicalItem::Type: {
                TScenarioGraphicalItem * scenarioBlock = (TScenarioGraphicalItem *)startItems.first();

                TScenarioGraphicalItemPort * firstSuitableItemPort = nullptr;
                for(TScenarioGraphicalItemPort * itemPort : scenarioBlock->getGraphicalItemPorts()) {
                    if(itemPort->portDirection() != TScenarioItemPort::TItemPortDirection::TOutputPort) {
                        continue;
                    }

                    if(!itemPort->hasGraphicalConnection()) {
                        return itemPort;
                    }

                    firstSuitableItemPort = itemPort;
                }

                return firstSuitableItemPort;
            }
            case TScenarioGraphicalItemPort::Type: {
                TScenarioGraphicalItemPort * startConnector = (TScenarioGraphicalItemPort *)startItems.first();
                return startConnector;
            }
            default:
                return nullptr;
        }
    }

    return nullptr;
}

TScenarioGraphicalItemPort * TScenarioScene::findLineEndPort(const QPointF & endPoint) {
    QList<QGraphicsItem *> endItems = items(endPoint);

    while(endItems.count() > 0) {
        if(endItems.first() == m_tmpLine ||
            (endItems.first()->type() != TScenarioGraphicalItem::Type && endItems.first()->type() != TScenarioGraphicalItemPort::Type)) {
            endItems.removeFirst();
            continue;
        }

        break;
    }

    if(endItems.count() > 0) {
        switch(endItems.first()->type()) {
            case TScenarioGraphicalItem::Type: {
                TScenarioGraphicalItem * scenarioBlock = (TScenarioGraphicalItem *)endItems.first();

                if(!m_tmpLineStartPort) {
                    return nullptr;
                }

                TScenarioGraphicalItemPort * firstSuitableItemPort = nullptr;
                for(TScenarioGraphicalItemPort * itemPort : scenarioBlock->getGraphicalItemPorts()) {
                    if(itemPort->portDirection() != TScenarioItemPort::TItemPortDirection::TInputPort) {
                        continue;
                    }

                    if(itemPort->portType() != m_tmpLineStartPort->portType()) {
                        continue;
                    }

                    if(!itemPort->hasGraphicalConnection()) {
                        return itemPort;
                    }

                    firstSuitableItemPort = itemPort;
                }

                return firstSuitableItemPort;
            }
            case TScenarioGraphicalItemPort::Type: {
                TScenarioGraphicalItemPort * endConnector = (TScenarioGraphicalItemPort *)endItems.first();
                return endConnector;
            }
            default:
                return nullptr;
        }
    }

    return nullptr;
}

bool TScenarioScene::isLineValidArrow() const {
    if(m_tmpLineStartPort == nullptr || m_tmpLineEndPort == nullptr) {
        return false;
    }

    if(m_tmpLineStartPort == m_tmpLineEndPort ||
        m_tmpLineStartPort->portType() != m_tmpLineEndPort->portType() ||
        m_tmpLineStartPort->parentItem() == m_tmpLineEndPort->parentItem()) {
        return false;
    }

    if(
        m_tmpLineStartPort->portDirection() == TScenarioItemPort::TItemPortDirection::TInputPort
        || m_tmpLineEndPort->portDirection() == TScenarioItemPort::TItemPortDirection::TOutputPort) {
        return false;
    }

    if(m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TFlowPort && m_tmpLineStartPort->hasGraphicalConnection()) {
        return false;
    }

    if(m_tmpLineEndPort->portType() == TScenarioItemPort::TItemPortType::TDataPort && m_tmpLineEndPort->hasGraphicalConnection()) {
        return false;
    }

    if(m_tmpLineEndPort->portType() == TScenarioItemPort::TItemPortType::TConnectionPort &&
        (m_tmpLineStartPort->hasGraphicalConnection() || m_tmpLineEndPort->hasGraphicalConnection())) {
        return false;
    }

    return true;
}

QPen TScenarioScene::evaluateLinePen() const {
    if(m_tmpLineStartPort == nullptr) {
        return QPen(Qt::red, 2);
    }

    if(m_tmpLineStartPort->portDirection() == TScenarioItemPort::TItemPortDirection::TInputPort ||
        (m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TFlowPort && m_tmpLineStartPort->hasGraphicalConnection()) ||
        (m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TConnectionPort && m_tmpLineStartPort->hasGraphicalConnection()))
    {
        return QPen(Qt::red, 2);
    }

    Qt::PenStyle style = m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TDataPort ? Qt::DashLine : Qt::SolidLine;

    if(m_tmpLineEndPort != nullptr)
    {
        if(m_tmpLineStartPort == m_tmpLineEndPort ||
            m_tmpLineStartPort->portType() != m_tmpLineEndPort->portType() ||
            m_tmpLineStartPort->parentItem() == m_tmpLineEndPort->parentItem()) {
            return QPen(Qt::red, 2, style);
        }

        if(m_tmpLineEndPort->portType() == TScenarioItemPort::TItemPortType::TDataPort && m_tmpLineEndPort->hasGraphicalConnection())
        {
            return QPen(Qt::red, 2, style);
        }

        if(m_tmpLineEndPort->portType() == TScenarioItemPort::TItemPortType::TConnectionPort && m_tmpLineEndPort->hasGraphicalConnection())
        {
            return QPen(Qt::red, 2, style);
        }

        if(m_tmpLineEndPort->portDirection() == TScenarioItemPort::TItemPortDirection::TOutputPort) {
            return QPen(Qt::red, 2, style);
        }
    }

    if(m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TFlowPort) {
        return QPen(FLOW_LINE_COLOR, 2, style);
    } else if(m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TDataPort) {
        return QPen(DATA_LINE_COLOR, 2, style);
    } else if(m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TConnectionPort) {
        return QPen(CONN_LINE_COLOR, 2, style);
    }

    return QPen(Qt::red, 2, style);
}
