#ifndef TSCENARIOITEM_H
#define TSCENARIOITEM_H

#include <QObject>
#include <QString>
#include <QList>
#include <QDataStream>
#include <QMetaType>
#include <QPoint>
#include <QDebug>
#include <qsize.h>

#include "tconfigparam.h"
#include "../project/tprojectmodel.h"
#include "tscenarioitemport.h"

#define TSCENARIOITEMVER "cz.cvut.fit.TraceXpert.TScenarioItem/0.2"

/*!
 * \brief The TScenario class represents a scenario item (called block in the GUI).
 *
 * The class represents a scenario item (called block in the GUI).
 * The whole class can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 *
 * The item has a name, description, position, type, title, subtitle, parameters and ports.
 * It has methods for execution purposes, status setting, copying and port management.
 * It also keeps state and has a factory method for instantiating items by class ("TItemClass").
 */

class TScenarioItem : public QObject {
    Q_OBJECT

public:
    enum class TLogLevel {
        TInfo,
        TSuccess,
        TWarning,
        TError
    };

    enum class TItemClass {
        TScenarioItem = 0,
        TScenarioFlowStartItem = 1,
        TScenarioFlowEndItem = 2,
        TScenarioFlowMergeItem = 3,
        TScenarioConditionItem = 4,
        TScenarioLogItem = 10,
        TScenarioIODeviceReadItem = 31,
        TScenarioIODeviceWriteItem = 32,
        TScenarioConstantValueItem = 40,
        TScenarioLoopItem = 50,
        TScenarioScopeSingleItem = 61,
        TScenarioScopeStartItem = 62,
        TScenarioScopeStopItem = 63,
        TScenarioDelayItem = 70,
        TScenarioProtocolEncodeItem = 80,
        TScenarioOutputFileItem = 90,
        TScenarioVariableReadItem = 100,
        TScenarioVariableWriteItem = 101,
        TScenarioScriptItem = 110,
        TScenarioAnalDeviceReadItem = 131,
        TScenarioAnalDeviceWriteItem = 132,
        TScenarioAnalDeviceActionItem = 133,
        TScenarioGraphWidgetItem = 140
    };

    enum class TItemAppearance {
        TDefault,
        TFlowStart,
        TFlowEnd,
        TFlowMerge,
        TCondition,
        TEmbeddedSubtitle
    };

    enum class TState {
        TOk = (int)TConfigParam::TState::TOk,
        TInfo = (int)TConfigParam::TState::TInfo,
        TWarning = (int)TConfigParam::TState::TWarning,
        TError = (int)TConfigParam::TState::TError,
        TBeingExecuted,
        TRuntimeInfo,
        TRuntimeWarning,
        TRuntimeError
    };

    TScenarioItem();
    TScenarioItem(const QString &name, const QString &description);
    virtual ~TScenarioItem();

    virtual TItemClass itemClass() const { return TItemClass::TScenarioItem; }

    TScenarioItem(const TScenarioItem &x);
    TScenarioItem & operator=(const TScenarioItem &x);
    bool operator==(const TScenarioItem *x) const;
    bool operator==(const TScenarioItem &x) const;

    friend QDataStream & operator<<(QDataStream &out, const TScenarioItem &x) {
        out << QString(TSCENARIOITEMVER);
        out << x.m_name;
        out << x.m_description;
        out << x.m_type;
        out << x.m_params;
        out << x.m_allowedDynamicParamNames;
        out << x.m_selectedDynamicParamNames;
        out << x.m_title;
        out << x.m_subtitle;
        out << x.m_position;
        out << x.m_state;
        out << x.m_stateMessage;

        out << x.m_itemPorts.size();
        for(TScenarioItemPort * itemPort : x.m_itemPorts) {
            out << *itemPort;
        }

        out << x.m_configWindowSize;

        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TScenarioItem * x) {
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TSCENARIOITEMVER))){
            qDeleteAll(x->m_itemPorts);
            x->m_itemPorts.clear();

            in >> x->m_name;
            in >> x->m_description;
            in >> x->m_type;
            in >> x->m_params;
            in >> x->m_allowedDynamicParamNames;
            in >> x->m_selectedDynamicParamNames;
            in >> x->m_title;
            in >> x->m_subtitle;
            in >> x->m_position;
            in >> x->m_state;
            in >> x->m_stateMessage;

            qsizetype itemPortCount;
            in >> itemPortCount;
            for(qsizetype i = 0; i < itemPortCount; i++) {
                TScenarioItemPort * itemPort = new TScenarioItemPort(x);
                in >> *itemPort;
                x->m_itemPorts.append(itemPort);
            }

            in >> x->m_configWindowSize;

        } else {
            qCritical("Failed deserializing scenario item: Wrong version or wrong data.");
            throw tr("Failed deserializing scenario item: Wrong version or wrong data.");
        }
        return in;
    }

    virtual TScenarioItem * copy() const;

    static TScenarioItem * createScenarioItemByClass(TScenarioItem::TItemClass itemClass);

    const QString & getName() const;
    const QString & getDescription() const;
    const QPointF & getPosition() const;
    const TItemAppearance & getType() const;

    void setName(const QString & value);
    void setDescription(const QString & value);
    void setPosition(const QPointF & value);
    void setType(TItemAppearance value);

    const QString & getTitle() const;
    const QString & getSubtitle() const;

    TConfigParam getParams() const;
    virtual TConfigParam setParams(TConfigParam params);
    virtual bool shouldUpdateParams(TConfigParam newParams);
    virtual void updateParams(bool paramValuesChanged = true);

    TScenarioItem::TState getState() const;
    const QString & getStateMessage() const;

    void setState(TState state);
    void setState(TState state, const QString &message);
    void resetState(bool resetOnlyRuntime = false);

    QList<TScenarioItemPort *> & getItemPorts();
    TScenarioItemPort * getItemPortByName(const QString & name);
    int getItemPortSideOrderByName(const QString & name);
    int getItemPortCountByDirection(TScenarioItemPort::TItemPortDirection direction);
    bool hasFlowInputPort();

    // methods for execution purposes
    void setProjectModel(TProjectModel * projectModel);

    void setDynamicParameters(const QHash<TScenarioItemPort *, QByteArray> & inputData);

    virtual bool                                    prepare();
    virtual bool                                    cleanup();

    virtual bool                                    supportsDirectExecution() const;
    virtual QHash<TScenarioItemPort *, QByteArray>  executeDirect(const QHash<TScenarioItemPort *, QByteArray> & inputData);

    virtual void                                    executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData);

    virtual void                                    stopExecution();
    virtual void                                    terminateExecution();

    virtual TScenarioItemPort *                     getPreferredOutputFlowPort();


    virtual const QString getIconResourcePath() const;

    QSize getConfigWindowSize();
    void setConfigWindowSize(QSize value);

    const QStringList & getAllowedDynamicParamNames() const;

    const QStringList & getSelectedDynamicParamNames() const;
    void setSelectedDynamicParamNames(const QStringList & selectedDynamicParamNames);

signals:
    void appearanceChanged();
    void stateChanged();
    void portsChanged();

    // signals for execution purposes
    void executionFinished(QHash<TScenarioItemPort *, QByteArray> outputData = {});

protected:
    virtual bool validateParamsStructure(TConfigParam params);

    void addFlowInputPort       (const QString & name, const QString & displayName = QString(), const QString & description = QString());
    void addFlowOutputPort      (const QString & name, const QString & displayName = QString(), const QString & description = QString());
    void addDataInputPort       (const QString & name, const QString & displayName = QString(), const QString & description = QString(), const QString & dataTypeHint = QString());
    void addDataOutputPort      (const QString & name, const QString & displayName = QString(), const QString & description = QString(), const QString & dataTypeHint = QString());
    void addConnectionInputPort (const QString & name, const QString & displayName = QString(), const QString & description = QString());
    void addConnectionOutputPort(const QString & name, const QString & displayName = QString(), const QString & description = QString());

    void removePort(const QString & name);
    bool verifyPortNameUnique(const QString & name);
    void sortItemPorts();

    static bool isParamValueDifferent(TConfigParam & paramsA, TConfigParam & paramsB, QString parameterName);

    void log(const QString & message, TLogLevel logLevel = TLogLevel::TInfo);

    TProjectModel * m_projectModel;
    QString m_preferredOutputFlowPortName;

    // serialized fields
    QString m_name;
    QString m_description;
    TItemAppearance m_type;

    TConfigParam m_params;
    QStringList m_allowedDynamicParamNames;
    QStringList m_selectedDynamicParamNames;

    QString m_title;
    QString m_subtitle;

    QPointF m_position;

    TState m_state = TState::TOk;
    QString m_stateMessage;

    QList<TScenarioItemPort *> m_itemPorts;

    // convenience serialized fields
    QSize m_configWindowSize = QSize(0, 0);
};

#endif // TSCENARIOITEM_H
