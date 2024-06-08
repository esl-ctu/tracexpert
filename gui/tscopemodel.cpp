#include "tscopemodel.h"

#include "tscopecontainer.h"
#include "tcomponentmodel.h"

TScopeModel::TScopeModel(TScope * scope, TScopeContainer * parent, bool manual)
    : TProjectItem(parent->model(), parent), TPluginUnitModel(scope, parent, manual), m_scope(scope)
{
    m_typeName = "scope";
}

TScopeModel::~TScopeModel()
{
    TScopeModel::deInit();
}

void TScopeModel::show()
{
    emit showRequested(this);
}

bool TScopeModel::init()
{
    if (isInit() || !TPluginUnitModel::init()) {
        return false;
    }

    m_isInit = true;

    m_repeat = false;
    m_stopping = false;

    if (collectorThread.isRunning())
        collectorThread.terminate();

    m_collector = new TScopeCollector(m_scope);
    m_collector->moveToThread(&collectorThread);

    connect(this, &TScopeModel::startDataCollection, m_collector, &TScopeCollector::collectData, Qt::ConnectionType::QueuedConnection);
    connect(m_collector, &TScopeCollector::dataCollected, this, &TScopeModel::dataCollected, Qt::ConnectionType::QueuedConnection);
    connect(m_collector, &TScopeCollector::collectionStopped, this, &TScopeModel::dataCollectionStopped, Qt::ConnectionType::QueuedConnection);
    connect(m_collector, &TScopeCollector::nothingCollected, this, &TScopeModel::noDataCollected, Qt::ConnectionType::QueuedConnection);
    connect(&collectorThread, &QThread::finished, m_collector, &QObject::deleteLater);

    collectorThread.start();

    emit initialized(this);

    return true;
}

bool TScopeModel::deInit()
{
    if (!isInit() || !TPluginUnitModel::deInit()) {
        return false;
    }

    collectorThread.quit();

    delete m_collector;

    m_isInit = false;

    emit deinitialized(this);

    return true;
}

bool TScopeModel::remove()
{
    TComponentModel * component = dynamic_cast<TComponentModel *>(TProjectItem::parent()->parent());
    if (!component)
        return false;

    return component->removeScope(this);
}

TConfigParam TScopeModel::setPostInitParams(const TConfigParam & param)
{
    stop();
    TConfigParam newParam = TPluginUnitModel::setPostInitParams(param);
    emit channelsStatusChanged();
    return newParam;
}

int TScopeModel::childrenCount() const
{
    return 0;
}

TProjectItem * TScopeModel::child(int row) const
{
    return nullptr;
}

void TScopeModel::bind(TCommon * unit)
{
    m_scope = dynamic_cast<TScope *>(unit);
    TPluginUnitModel::bind(m_scope);
}

void TScopeModel::release()
{
    m_scope = nullptr;
    TPluginUnitModel::release();
}

QList<TScope::TChannelStatus> TScopeModel::channelsStatus()
{
    return m_scope->getChannelsStatus();
}

TScope::TTriggerStatus TScopeModel::triggerStatus()
{
    return m_scope->getTriggerStatus();
}


void TScopeModel::run()
{
    run(true);
}

void TScopeModel::runSingle()
{
    run(false);
}

void TScopeModel::stop()
{
    bool ok;
    m_stopping = true;

    m_scope->stop(&ok);

    if (!ok) {
        emit stopFailed();
        m_stopping = false;
        return;
    }

    emit stopped();
}

void TScopeModel::dataCollected(size_t traces, size_t samples, TScope::TSampleType type, QList<quint8 *> buffers, bool overvoltage)
{
    emit tracesDownloaded(traces, samples, type, buffers, overvoltage);

    if (m_repeat && !m_stopping) { // discuss addition of !m_stopping
        run(m_repeat);
    }
    else {
        stop();
    }
}

void TScopeModel::dataCollectionStopped()
{
    if (!m_stopping) {
        emit downloadFailed();
    }
}

void TScopeModel::noDataCollected()
{
    emit tracesEmpty();
}

void TScopeModel::run(bool repeat)
{
    size_t bufferSize;
    bool ok;

    m_repeat = repeat;
    m_stopping = false;

    m_scope->run(&bufferSize, &ok);

    if (!ok) {
        emit runFailed();
        return;
    }

    emit startDataCollection(bufferSize);
}

TScopeCollector::TScopeCollector(TScope * scope, QObject * parent)
    : QObject(parent), m_scope(scope)
{

}

void TScopeCollector::collectData(size_t bufferSize)
{
    QList<TScope::TChannelStatus> status = m_scope->getChannelsStatus();

    QList<quint8 *> buffers;
    TScope::TSampleType type;
    size_t samplesPerTrace;
    size_t traces;
    bool overload = false;
    int channels = 0;

    for (int i = 0; i < status.count(); i++) {
        quint8 * buffer = nullptr;

        if (status[i].isEnabled()) {
            bool overloadSingle;
            buffer = new uint8_t[bufferSize];

            m_scope->downloadSamples(status[i].getIndex(), buffer, bufferSize, &type, &samplesPerTrace, &traces, &overloadSingle);

            if (!traces) {
                for (int j = 0; j < buffers.count(); j++)
                    if (buffers[j])
                        delete [] buffers[j];
                delete [] buffer;

                emit collectionStopped();
                return;
            }

            overload = overload || overloadSingle;
            channels++;
        }

        buffers.append(buffer);
    }

    if (channels) {
        emit dataCollected(traces, samplesPerTrace, type, buffers, overload);
    }
    else {
        emit nothingCollected();
    }
}
