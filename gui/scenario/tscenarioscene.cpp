#include "tscenarioscene.h"
#include "tscenariographicalitemport.h"
#include "tscenariographicalconnection.h"
#include "tscenariographicalitem.h"

#include <QGraphicsSceneMouseEvent>

TScenarioScene::TScenarioScene(TProjectModel * projectModel, QObject * parent) :
    QGraphicsScene(parent),
    m_mode(MousePointer),
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

void TScenarioScene::setMode(Mode mode)
{
    cancelInsertMode();
    m_mode = mode;
}

TScenarioScene::Mode TScenarioScene::mode() {
    return m_mode;
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

        m_scenario->removeItem(graphicalItem->getScenarioItem(), &ok);

        if(!ok) {
            qWarning("Failed to remove item from scenario.");
        }
    }
    else if(item->type() == TScenarioGraphicalConnection::Type) {
        TScenarioGraphicalConnection * graphicalConnection = (TScenarioGraphicalConnection *)item;
        graphicalConnection->startItem()->removeGraphicalConnection(graphicalConnection);
        graphicalConnection->endItem()->removeGraphicalConnection(graphicalConnection);

        m_scenario->removeConnection(graphicalConnection->getScenarioConnection(), &ok);

        if(!ok) {
            qWarning("Failed to remove connection from scenario.");
        }
    }

    QGraphicsScene::removeItem(item);
}

void TScenarioScene::removeSelectedItems() {
    QList<QGraphicsItem *> selectedItems = this->selectedItems();

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
    if(m_insertedBlockInstance != nullptr) {
        removeItem(m_insertedBlockInstance);
    }

    m_insertedBlockInstance = insertedBlockInstance;    
    m_insertedBlockInstance->setOpacity(0); // hide before cursor first over scene
    addItem(m_insertedBlockInstance);

    m_mode = InsertItem;
}

void TScenarioScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent) {
    if(mouseEvent->button() == Qt::LeftButton) {
        switch (m_mode) {
            case InsertItem:
                bool ok;
                m_scenario->addItem(m_insertedBlockInstance->getScenarioItem(), &ok);                

                if(ok) {
                    m_insertedBlockInstance->getScenarioItem()->setProjectModel(m_projectModel);
                    m_insertedBlockInstance->getScenarioItem()->updateParams(false);
                    m_insertedBlockInstance->setScene(this);
                    m_insertedBlockInstance->setOpacity(1.0);
                    m_insertedBlockInstance->setZValue(1);
                    m_insertedBlockInstance = nullptr;
                    m_mode = MousePointer;

                    qDebug("Item added to scenario.");
                    emit itemInserted(m_insertedBlockInstance);
                    break;
                }

                qDebug("Failed to add item to scenario.");
                break;
            case InsertLine: {
                    m_tmpLineStartPort = findLineStartPort(mouseEvent->scenePos());
                    QPointF lineStartPoint = m_tmpLineStartPort == nullptr ? mouseEvent->scenePos() : m_tmpLineStartPort->scenePos();
                    m_tmpLine = new QGraphicsLineItem(QLineF(lineStartPoint, lineStartPoint));
                    m_tmpLine->setPen(QPen(Qt::black, 2));
                    m_tmpLine->setZValue(2);
                    addItem(m_tmpLine);
                    break;
                }
            case MouseDrag:
                break;
            default:
                QGraphicsScene::mousePressEvent(mouseEvent);
                break;
        }
    }
    else if(mouseEvent->button() == Qt::RightButton) {
        cancelInsertMode();
        m_mode = MousePointer;
    }
    else {
        QGraphicsScene::mousePressEvent(mouseEvent);
    }
}

void TScenarioScene::cancelInsertMode() {
    switch (m_mode) {
        case InsertItem:
            if(m_insertedBlockInstance != nullptr) {
                removeItem(m_insertedBlockInstance);
                delete m_insertedBlockInstance;
                m_insertedBlockInstance = nullptr;
                emit itemInsertCanceled();
            }
            break;
        case InsertLine:
            if (m_tmpLine != nullptr) {
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
    if(m_mode == InsertItem) {
        m_insertedBlockInstance->setOpacity(0.75);
        m_insertedBlockInstance->setPos(mouseEvent->scenePos());
    }
    else if(m_mode == InsertLine && m_tmpLine != nullptr) {
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
    if (m_tmpLine != nullptr && m_mode == InsertLine) {
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

    if(m_tmpLineStartPort->hasGraphicalConnection()) {
        return false;
    }

    return true;
}

QPen TScenarioScene::evaluateLinePen() const {
    if(m_tmpLineStartPort == nullptr) {
        return QPen(Qt::red, 2);
    }

    if(m_tmpLineStartPort->portDirection() == TScenarioItemPort::TItemPortDirection::TInputPort || m_tmpLineStartPort->hasGraphicalConnection()) {
        return QPen(Qt::red, 2);
    }

    Qt::PenStyle style = m_tmpLineStartPort->portType() == TScenarioItemPort::TItemPortType::TFlowPort ? Qt::SolidLine : Qt::DashLine;

    if(m_tmpLineEndPort != nullptr)
    {
        if(m_tmpLineStartPort == m_tmpLineEndPort ||
            m_tmpLineStartPort->portType() != m_tmpLineEndPort->portType() ||
            m_tmpLineStartPort->parentItem() == m_tmpLineEndPort->parentItem()) {
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
    }

    return QPen(Qt::red, 2, style);
}
