#include "tanaltestingdevice.h"

#include <QtGlobal>

#include "tanaltestingaction.h"
#include "tanaltestinginputstream.h"
#include "tanaltestingoutputstream.h"

TAnalTestingDevice::TAnalTestingDevice() {
    m_preInitParams = TConfigParam("Anal configuration", "", TConfigParam::TType::TDummy, "");
    TConfigParam traceLength = TConfigParam("Trace length (samples)", "100", TConfigParam::TType::TInt, "The number of samples per data trace");
    m_preInitParams.addSubParam(traceLength);
}

TAnalTestingDevice::~TAnalTestingDevice() {

}

QString TAnalTestingDevice::getName() const {
    return QString("Anal testing device");
}

QString TAnalTestingDevice::getInfo() const {
    return QString("Default device for testing analytical device-related features");
}


TConfigParam TAnalTestingDevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TAnalTestingDevice::setPreInitParams(TConfigParam params) {
    bool iok;

    m_preInitParams = params;
    m_preInitParams.resetState(true);

    TConfigParam * traceLengthParam = m_preInitParams.getSubParamByName("Trace length (samples)", &iok);
    if(!iok) {
        qCritical("Trace length parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }

    m_traceLength = traceLengthParam->getValue().toInt(&iok);
    if (!iok || m_traceLength < 1) {
        traceLengthParam->setState(TConfigParam::TState::TError, "Trace length must be positive integer");
    }
    return m_preInitParams;
}

void TAnalTestingDevice::init(bool *ok) {
    if (ok != nullptr) *ok = true;

    m_analActions.append(new TAnalTestingAction("Add", "Adds traces in two sets", [=](){ processData(false); }));
    m_analActions.append(new TAnalTestingAction("Subtract", "Subtract traces in two sets", [=](){ processData(true); }));

    m_analInputStreams.append(new TAnalTestingInputStream("Result", "Stream of results of last used action", [=](uint8_t * buffer, size_t length){ return getData(buffer, length); }));

    m_analOutputStreams.append(new TAnalTestingOutputStream("First set", "Stream of traces in first set", [=, &set = m_firstSet](const uint8_t * buffer, size_t length){ return fillData(buffer, length, set); }));
    m_analOutputStreams.append(new TAnalTestingOutputStream("Second set", "Stream of traces in second set", [=, &set = m_secondSet](const uint8_t * buffer, size_t length){ return fillData(buffer, length, set); }));
}

void TAnalTestingDevice::deInit(bool *ok) {
    if (ok != nullptr) *ok = true;

    for (int i = 0; i < m_analActions.length(); i++) {
        delete m_analActions[i];
    }
    m_analActions.clear();

    for (int i = 0; i < m_analInputStreams.length(); i++) {
        delete m_analInputStreams[i];
    }
    m_analInputStreams.clear();

    for (int i = 0; i < m_analOutputStreams.length(); i++) {
        delete m_analOutputStreams[i];
    }
    m_analOutputStreams.clear();

    if (m_data) {
        delete [] m_data;
        m_data = nullptr;
    }
}

TConfigParam TAnalTestingDevice::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TAnalTestingDevice::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

QList<TAnalAction *> TAnalTestingDevice::getActions() const
{
    return m_analActions;
}

QList<TAnalInputStream *> TAnalTestingDevice::getInputDataStreams() const
{
    return m_analInputStreams;
}

QList<TAnalOutputStream *> TAnalTestingDevice::getOutputDataStreams() const
{
    return m_analOutputStreams;
}

bool TAnalTestingDevice::isBusy() const
{
    return false;
}

size_t TAnalTestingDevice::fillData(const uint8_t * buffer, size_t length, QList<QList<uint8_t> *> & set)
{
    size_t position = 0;

    while (position < length) {
        if (set.empty() || (size_t)set.last()->length() >= m_traceLength)
            set.append(new QList<uint8_t>());

        QList<uint8_t> * current = set.last();

        size_t toAdd = qMin(m_traceLength - current->length(), length - position);

        for (; toAdd > 0; toAdd--, position++) {
            current->append(buffer[position]);
        }
    }

    return position;
}

void TAnalTestingDevice::processData(bool subtract)
{
    int max = qMax(m_firstSet.length(), m_secondSet.length());
    m_length = max * m_traceLength;
    m_position = 0;

    if (m_data) {
        delete m_data;
        m_data = nullptr;
    }

    m_data = new int[m_length];

    for (int i = 0; i < max; i++) {
        for (int j = 0; j < m_traceLength; j++) {
            int first = m_firstSet.length() > i && m_firstSet[i]->length() > j ? m_firstSet.at(i)->at(j) : 0 ;
            int second = m_secondSet.length() > i && m_secondSet[i]->length() > j ? m_secondSet.at(i)->at(j) : 0 ;

            m_data[i * m_traceLength + j] = subtract ? first - second : first + second;
        }
    }
}

size_t TAnalTestingDevice::getData(uint8_t * buffer, size_t length)
{
    if (!m_data)
        return 0;

    int i;

    for (i = 0; m_position < m_length && i < length; i++, m_position++) {
        buffer[i] = m_data[m_position];
    }

    return i;
}
