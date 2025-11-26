#include <QObject>
#include <QString>
#include <QList>
#include <QDataStream>
#include <QMetaType>
#include <QPoint>
#include <QDebug>

#include "scenario_items/tscenariobasicitems.h"
#include "scenario_items/tscenarioconstantvalueitem.h"
#include "scenario_items/tscenariodelayitem.h"
#include "scenario_items/tscenariographwidgetitem.h"
#include "scenario_items/tscenarioiodevicereaditem.h"
#include "scenario_items/tscenarioiodevicewriteitem.h"
#include "scenario_items/tscenariologitem.h"
#include "scenario_items/tscenarioloopitem.h"
#include "scenario_items/tscenariooutputfileitem.h"
#include "scenario_items/tscenarioscopesingleitem.h"
#include "scenario_items/tscenarioscopestartitem.h"
#include "scenario_items/tscenarioscopestopitem.h"
#include "scenario_items/tscenarioprotocolencodeitem.h"
#include "scenario_items/tscenariovariablereaditem.h"
#include "scenario_items/tscenariovariablewriteitem.h"
#include "scenario_items/tscenarioscriptitem.h"
#include "../project/tprojectmodel.h"
#include "scenario_items/tscenarioanaldevicereaditem.h"
#include "scenario_items/tscenarioanaldevicewriteitem.h"
#include "scenario_items/tscenarioanaldeviceactionitem.h"
#include "tscenarioitem.h"

#include "tconfigparam.h"
#include "tscenarioitemport.h"


TScenarioItem::TScenarioItem() { }

TScenarioItem::TScenarioItem(const QString &name, const QString &description) :
    m_name(name),
    m_description(description),
    m_type(TItemAppearance::TDefault)
{ }

TScenarioItem::~TScenarioItem() {
    qDeleteAll(m_itemPorts);
    m_itemPorts.clear();
}

TScenarioItem::TScenarioItem(const TScenarioItem &x): // TScenarioItemExecutionIterface(x),
    m_name(x.m_name),
    m_description(x.m_description),
    m_type(x.m_type),
    m_params(x.m_params),
    m_title(x.m_title),
    m_subtitle(x.m_subtitle),
    m_position(x.m_position),
    m_state(x.m_state),
    m_stateMessage(x.m_stateMessage)
{

    // deep copy ports
    for(TScenarioItemPort * port : x.m_itemPorts) {
        TScenarioItemPort * newPort = new TScenarioItemPort(*port);
        newPort->setParentItem(this);
        m_itemPorts.append(newPort);
    }
}

TScenarioItem & TScenarioItem::operator=(const TScenarioItem &x) {
    if(&x != this){
        m_name = x.m_name;
        m_description = x.m_description;
        m_type = x.m_type;
        m_params = x.m_params;
        m_title = x.m_title;
        m_subtitle = x.m_subtitle;
        m_state = x.m_state;
        m_stateMessage = x.m_stateMessage;
        m_position = x.m_position;

        // delete previous ports of this object...
        qDeleteAll(m_itemPorts);
        m_itemPorts.clear();

        // deep copy ports
        for(TScenarioItemPort * port : x.m_itemPorts) {
            TScenarioItemPort * newPort = new TScenarioItemPort(*port);
            newPort->setParentItem(this);
            m_itemPorts.append(newPort);
        }
    }
    return *this;
}

bool TScenarioItem::operator==(const TScenarioItem * x) const{
    return (m_name == x->m_name);
}

bool TScenarioItem::operator==(const TScenarioItem &x) const {
    return (m_name == x.m_name);
}

TScenarioItem * TScenarioItem::copy() const {
    return new TScenarioItem(*this);
}

TScenarioItem * TScenarioItem::createScenarioItemByClass(TScenarioItem::TItemClass itemClass) {
    switch(itemClass) {
        case TScenarioItem::TItemClass::TScenarioProtocolEncodeItem:
            return new TScenarioProtocolEncodeItem();
        case TScenarioItem::TItemClass::TScenarioDelayItem:
            return new TScenarioDelayItem();
        case TScenarioItem::TItemClass::TScenarioConstantValueItem:
            return new TScenarioConstantValueItem();
        case TScenarioItem::TItemClass::TScenarioIODeviceReadItem:
            return new TScenarioIODeviceReadItem();
        case TScenarioItem::TItemClass::TScenarioIODeviceWriteItem:
            return new TScenarioIODeviceWriteItem();
        case TScenarioItem::TItemClass::TScenarioLogItem:
            return new TScenarioLogItem();
        case TScenarioItem::TItemClass::TScenarioFlowStartItem:
            return new TScenarioFlowStartItem();
        case TScenarioItem::TItemClass::TScenarioFlowEndItem:
            return new TScenarioFlowEndItem();
        case TScenarioItem::TItemClass::TScenarioFlowMergeItem:
            return new TScenarioFlowMergeItem();
        case TScenarioItem::TItemClass::TScenarioConditionItem:
            return new TScenarioConditionItem();
        case TScenarioItem::TItemClass::TScenarioLoopItem:
            return new TScenarioLoopItem();
        case TScenarioItem::TItemClass::TScenarioScopeSingleItem:
            return new TScenarioScopeSingleItem();
        case TScenarioItem::TItemClass::TScenarioScopeStartItem:
            return new TScenarioScopeStartItem();
        case TScenarioItem::TItemClass::TScenarioScopeStopItem:
            return new TScenarioScopeStopItem();
        case TScenarioItem::TItemClass::TScenarioOutputFileItem:
            return new TScenarioOutputFileItem();
        case TScenarioItem::TItemClass::TScenarioVariableReadItem:
            return new TScenarioVariableReadItem();
        case TScenarioItem::TItemClass::TScenarioVariableWriteItem:
            return new TScenarioVariableWriteItem();
        case TScenarioItem::TItemClass::TScenarioScriptItem:
            return new TScenarioScriptItem();
        case TScenarioItem::TItemClass::TScenarioAnalDeviceReadItem:
            return new TScenarioAnalDeviceReadItem();
        case TScenarioItem::TItemClass::TScenarioAnalDeviceWriteItem:
            return new TScenarioAnalDeviceWriteItem();
        case TScenarioItem::TItemClass::TScenarioAnalDeviceActionItem:
            return new TScenarioAnalDeviceActionItem();
        case TScenarioItem::TItemClass::TScenarioGraphWidgetItem:
            return new TScenarioCreateGraphItem();
        case TScenarioItem::TItemClass::TScenarioItem:
            return new TScenarioItem();
        default:
            return nullptr;
    }
}

const QString & TScenarioItem::getName() const {
    return m_name;
}

const QString & TScenarioItem::getDescription() const {
    return m_description;
}

const QPointF & TScenarioItem::getPosition() const {
    return m_position;
}

const TScenarioItem::TItemAppearance & TScenarioItem::getType() const {
    return m_type;
}

TConfigParam TScenarioItem::getParams() const {
    return m_params;
}

bool TScenarioItem::shouldUpdateParams(TConfigParam newParams) {
    return false;
}

void TScenarioItem::updateParams(bool paramValuesChanged) {}

const QString & TScenarioItem::getTitle() const {
    return m_title.isEmpty() ? m_name : m_title;
}
const QString & TScenarioItem::getSubtitle() const {
    return m_subtitle;
}

void TScenarioItem::setName(const QString & value) {
    m_name = value;
}

void TScenarioItem::setDescription(const QString & value) {
    m_description = value;
}

void TScenarioItem::setPosition(const QPointF & value) {
    m_position = value;
}

void TScenarioItem::setType(TItemAppearance value) {
    m_type = value;
}

TConfigParam TScenarioItem::setParams(TConfigParam params) {
    m_params = params;
    return m_params;
}

TScenarioItem::TState TScenarioItem::getState() const {
    return m_state;
}

const QString & TScenarioItem::getStateMessage() const {
    return m_stateMessage;
}

void TScenarioItem::setState(TState state){
    if(state >= m_state) {
        m_state = state;
        emit stateChanged();
    }
}

void TScenarioItem::setState(TState state, const QString &message){
    if(state >= m_state) {
        m_state = state;
        m_stateMessage = message;
        emit stateChanged();
    }
}

void TScenarioItem::resetState(bool resetOnlyRuntime){
    if(resetOnlyRuntime) {
        if( m_state != TState::TBeingExecuted &&
            m_state != TState::TRuntimeInfo &&
            m_state != TState::TRuntimeWarning &&
            m_state != TState::TRuntimeError)
        {
            return;
        }
    }

    m_state = TState::TOk;
    m_stateMessage = "";
    emit stateChanged();
}

QList<TScenarioItemPort *> & TScenarioItem::getItemPorts() {
    return m_itemPorts;
}

TScenarioItemPort * TScenarioItem::getItemPortByName(const QString & name) {
    for(TScenarioItemPort * p : m_itemPorts) {
        if (p->getName() == name) {
            return p;
        }
    }

    return nullptr;
}

int TScenarioItem::getItemPortCountByDirection(TScenarioItemPort::TItemPortDirection direction) {
    int count = 0;

    for(TScenarioItemPort * p : m_itemPorts) {
        if (p->getDirection() == direction) {
            count++;
        }
    }

    return count;
}

int TScenarioItem::getItemPortSideOrderByName(const QString & name) {
    int index = 0;
    TScenarioItemPort * lastItemPort = nullptr;
    for(TScenarioItemPort * p : m_itemPorts) {
        if (lastItemPort && lastItemPort->getDirection() != p->getDirection()) {
            index = 0;
        }

        if (p->getName() == name) {
            return index;
        }

        lastItemPort = p;
        index++;
    }

    return -1;
}


void TScenarioItem::addFlowInputPort(const QString & name, const QString & displayName, const QString & description) {
    if(!verifyPortNameUnique(name)) {
        return;
    }

    m_itemPorts.append(
        new TScenarioItemPort(name,
                              TScenarioItemPort::TItemPortType::TFlowPort,
                              TScenarioItemPort::TItemPortDirection::TInputPort,
                              this, displayName, description)
        );
    sortItemPorts();
}

void TScenarioItem::addFlowOutputPort(const QString & name, const QString & displayName, const QString & description) {
    if(!verifyPortNameUnique(name)) {
        return;
    }

    m_itemPorts.append(
        new TScenarioItemPort(name,
                              TScenarioItemPort::TItemPortType::TFlowPort,
                              TScenarioItemPort::TItemPortDirection::TOutputPort,
                              this, displayName, description)
        );
    sortItemPorts();
}

void TScenarioItem::addDataInputPort(const QString & name, const QString & displayName, const QString & description) {
    if(!verifyPortNameUnique(name)) {
        return;
    }

    m_itemPorts.append(
        new TScenarioItemPort(name,
                              TScenarioItemPort::TItemPortType::TDataPort,
                              TScenarioItemPort::TItemPortDirection::TInputPort,
                              this, displayName, description)
        );
    sortItemPorts();
}

void TScenarioItem::addDataOutputPort(const QString & name, const QString & displayName, const QString & description) {
    if(!verifyPortNameUnique(name)) {
        return;
    }

    m_itemPorts.append(
        new TScenarioItemPort(name,
                              TScenarioItemPort::TItemPortType::TDataPort,
                              TScenarioItemPort::TItemPortDirection::TOutputPort,
                              this, displayName, description)
        );
    sortItemPorts();
}

void TScenarioItem::addConnectionInputPort(const QString & name, const QString & displayName, const QString & description) {
    if(!verifyPortNameUnique(name)) {
        return;
    }

    m_itemPorts.append(
        new TScenarioItemPort(name,
                              TScenarioItemPort::TItemPortType::TConnectionPort,
                              TScenarioItemPort::TItemPortDirection::TInputPort,
                              this, displayName, description)
        );
    sortItemPorts();
}

void TScenarioItem::addConnectionOutputPort(const QString & name, const QString & displayName, const QString & description) {
    if(!verifyPortNameUnique(name)) {
        return;
    }

    m_itemPorts.append(
        new TScenarioItemPort(name,
                              TScenarioItemPort::TItemPortType::TConnectionPort,
                              TScenarioItemPort::TItemPortDirection::TOutputPort,
                              this, displayName, description)
        );
    sortItemPorts();
}

void TScenarioItem::removePort(const QString & name) {
    QList<TScenarioItemPort *>::iterator it = m_itemPorts.begin();
    while (it != m_itemPorts.end()) {
        if ((*it)->getName() == name) {
            const TScenarioItemPort * port = *it;
            it = m_itemPorts.erase(it);
            delete port;
        }
        else {
            ++it;
        }
    }

    emit portsChanged();
}

bool TScenarioItem::hasFlowInputPort() {
    for(TScenarioItemPort * p : m_itemPorts) {
        if (p->getDirection() == TScenarioItemPort::TItemPortDirection::TInputPort &&
            p->getType() == TScenarioItemPort::TItemPortType::TFlowPort) {
            return true;
        }
    }

    return false;
}

bool TScenarioItem::verifyPortNameUnique(const QString & name) {
    for(TScenarioItemPort * p : m_itemPorts) {
        if (p->getName() == name) {
            return false;
        }
    }

    return true;
}

void TScenarioItem::sortItemPorts() {
    std::sort(m_itemPorts.begin(), m_itemPorts.end(),
        [=] (TScenarioItemPort * ip1, TScenarioItemPort * ip2)->bool {
            int ip1Rank = (int)ip1->getDirection() * 2 + (int)ip1->getType();
            int ip2Rank = (int)ip2->getDirection() * 2 + (int)ip2->getType();
            return ip1Rank < ip2Rank;
        }
    );

    emit portsChanged();
}

bool TScenarioItem::isParamValueDifferent(TConfigParam & paramsA, TConfigParam & paramsB, QString parameterName) {
    bool iok;
    TConfigParam * paramA = paramsA.getSubParamByName(parameterName, &iok);

    if(!iok)
        return true;

    TConfigParam * paramB = paramsB.getSubParamByName(parameterName, &iok);

    if(!iok)
        return true;

    return paramA->getValue() != paramB->getValue();
}

bool TScenarioItem::prepare() {
    return true;
}

QHash<TScenarioItemPort *, QByteArray> TScenarioItem::executeDirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) {
    if(!supportsDirectExecution()) {
        qWarning("Scenario item was executed using the wrong method!");
    }

    qWarning("Scenario item was executed, but execute method is not implemented!");
    return QHash<TScenarioItemPort *, QByteArray>();
}

void TScenarioItem::executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) {
    if(supportsDirectExecution()) {
        qWarning("Scenario item was executed using the wrong method!");
    }

    qWarning("Scenario item was executed, but execute method is not implemented!");
    emit executionFinished();
}

/**
 * This method is meant to abort execution,
 * meaning it gives an opportunity to the item to cancel its execution gracefully.
 *
 * IMPLEMENTATION NOTE:
 * The ScenarioExecutor still *expects to receive the executionFinished signal* after this method is called.
 * Indicate that cancelling was successful by emitting the signal!
 */
void TScenarioItem::stopExecution() { }

/**
 * This method is meant to terminate execution forcefully.
 * There is no opportunity to take time executing this method.
 *
 * IMPLEMENTATION NOTE:
 * The ScenarioExecutor does not expect the executionFinished signal after this method is called.
 */
void TScenarioItem::terminateExecution() { }

TScenarioItemPort * TScenarioItem::getPreferredOutputFlowPort() {
    return getItemPortByName(m_preferredOutputFlowPortName);
}

bool TScenarioItem::cleanup() {
    return true;
}

void TScenarioItem::setProjectModel(TProjectModel * projectModel) {
    m_projectModel = projectModel;
}

bool TScenarioItem::supportsDirectExecution() const {
    return true;
}

const QString TScenarioItem::getIconResourcePath() const {
    return "";
}

QSize TScenarioItem::getConfigWindowSize() {
    return m_configWindowSize;
}

void TScenarioItem::setConfigWindowSize(QSize value) {
    m_configWindowSize = value;
}

void TScenarioItem::log(const QString & message, TLogLevel logLevel) {
    QString prefixedMessage = QString("[%1] %2").arg(m_title.isEmpty() ? m_name : m_title).arg(message);

    switch(logLevel) {
        case TLogLevel::TError:
        case TLogLevel::TWarning:
            qWarning().noquote() << prefixedMessage;
            break;
        case TLogLevel::TInfo:
        case TLogLevel::TSuccess:
        default:
            qInfo().noquote() << prefixedMessage;
            break;
    }
}

bool TScenarioItem::validateParamsStructure(TConfigParam params) {
    return true;
}
