#include "tttestdevice.h"

#include <QtGlobal>

#include "tttestaction.h"
#include "tttestinputstream.h"
#include "tttestoutputstream.h"

TTTestDevice::TTTestDevice() {
    m_preInitParams = TConfigParam("Welch's t-test configuration", "", TConfigParam::TType::TDummy, "");
    TConfigParam traceLength = TConfigParam("Trace length (in samples)", "1000", TConfigParam::TType::TUInt, "The number of samples per data trace");
    m_preInitParams.addSubParam(traceLength);
    TConfigParam numberOfClasses = TConfigParam("Number of classes", "2", TConfigParam::TType::TUInt, "The number of classes to perform t-test between");
    m_preInitParams.addSubParam(numberOfClasses);
    TConfigParam traceType = TConfigParam("Data type", "Unsigned 8 bit", TConfigParam::TType::TEnum, "Data type of the samples");
    traceType.addEnumValue("Unsigned 8 bit");
    traceType.addEnumValue("Signed 8 bit");
    traceType.addEnumValue("Unsigned 16 bit");
    traceType.addEnumValue("Signed 16 bit");
    traceType.addEnumValue("Unsigned 32 bit");
    traceType.addEnumValue("Signed 32 bit");
    traceType.addEnumValue("Real 32 bit (float)");
    traceType.addEnumValue("Real 64 bit (double)");
    m_preInitParams.addSubParam(traceType);
    TConfigParam order = TConfigParam("Maximum order", "1", TConfigParam::TType::TUInt, "Maximum order of the t-test");
    m_preInitParams.addSubParam(order);

}

TTTestDevice::~TTTestDevice() {
    (*this).TTTestDevice::deInit();
}

QString TTTestDevice::getName() const {
    return QString("Welch's t-test computational device");
}

QString TTTestDevice::getInfo() const {
    return QString("Default device for t-test leakage assessment features");
}


TConfigParam TTTestDevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TTTestDevice::setPreInitParams(TConfigParam params) {
    bool iok;

    m_preInitParams = params;
    m_preInitParams.resetState(true);

    TConfigParam * traceLengthParam = m_preInitParams.getSubParamByName("Trace length (in samples)", &iok);
    if(!iok) {
        qCritical("Trace length parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_traceLength = traceLengthParam->getValue().toUInt(&iok);
    if (!iok || m_traceLength < 1) {
        traceLengthParam->setState(TConfigParam::TState::TError, "Trace length must be positive integer");
    }

    TConfigParam * numberOfClassesParam = m_preInitParams.getSubParamByName("Number of classes", &iok);
    if(!iok) {
        qCritical("Number of classes parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_numberOfClasses = numberOfClassesParam->getValue().toUInt(&iok);
    if (!iok || m_numberOfClasses < 2) {
        numberOfClassesParam->setState(TConfigParam::TState::TError, "Number of classes must be 2 or greater.");
    }

    TConfigParam * traceTypeParam = m_preInitParams.getSubParamByName("Data type", &iok);
    if(!iok) {
        qCritical("Trace data type parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_traceType = traceTypeParam->getValue();

    TConfigParam * orderParam = m_preInitParams.getSubParamByName("Maximum order", &iok);
    if(!iok) {
        qCritical("Maximum order parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_order = orderParam->getValue().toUInt(&iok);
    if (!iok || m_order < 1) {
        orderParam->setState(TConfigParam::TState::TError, "Maximum order must be 1 or greater.");
    }

    return m_preInitParams;
}

void TTTestDevice::init(bool *ok) {    

    m_analActions.append(new TTTestAction("Reset (delete all data)", "", [=](){ resetContexts(); }));

    for(int i = 0; i < m_numberOfClasses; i++){
        QString streamName = QString("Class %1").arg(i);
        m_analOutputStreams.append(new TTTestOutputStream(streamName, "Stream of traces in respective class", [=, classNo = i](const uint8_t * buffer, size_t length){ return addTraces(buffer, length, classNo); }));
    }

    for(int order = 1; order <= m_order; order++){
        for (int i = 0; i < m_numberOfClasses; i++) {
            for (int j = i + 1; j < m_numberOfClasses; j++) {

                QString streamName = QString("%1-order t-vals %2 vs %3").arg(order).arg(i).arg(j);
                m_analInputStreams.append(new TTTestInputStream(streamName, "Stream of t-values", [=, class1 = i, class2 = j, order0 = order](uint8_t * buffer, size_t length){ return getTValues(buffer, length, class1, class2, order0); }));

            }
        }
    }

    for(int i = 0; i < m_numberOfClasses; i++){
        m_contexts.append(new SICAK::Moments2DContext<qreal>(m_traceLength, 0, 1, 0, 2*m_order, 0, 0));
        m_contexts[i]->reset();
    }

    if (ok != nullptr) *ok = true;

}

void TTTestDevice::deInit(bool *ok) {

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

    for (int i = 0; i < m_contexts.length(); i++) {
        delete m_contexts[i];
    }
    m_contexts.clear();

    if (m_data) {
        delete [] m_data;
        m_data = nullptr;
    }

    if (ok != nullptr) *ok = true;
}

TConfigParam TTTestDevice::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TTTestDevice::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

QList<TAnalAction *> TTTestDevice::getActions() const
{
    return m_analActions;
}

QList<TAnalInputStream *> TTTestDevice::getInputDataStreams() const
{
    return m_analInputStreams;
}

QList<TAnalOutputStream *> TTTestDevice::getOutputDataStreams() const
{
    return m_analOutputStreams;
}

bool TTTestDevice::isBusy() const
{
    return false;
}

size_t TTTestDevice::addTraces(const uint8_t * buffer, size_t length, size_t classNo){

    size_t sampleSize;

    if(m_traceType == "Unsigned 8 bit") {
        sampleSize = sizeof(uint8_t);
    } else if(m_traceType == "Signed 8 bit"){
        sampleSize = sizeof(int8_t);
    } else if(m_traceType == "Unsigned 16 bit"){
        sampleSize = sizeof(uint16_t);
    } else if(m_traceType == "Signed 16 bit"){
        sampleSize = sizeof(int16_t);
    } else if(m_traceType == "Unsigned 32 bit"){
        sampleSize = sizeof(uint32_t);
    } else if(m_traceType == "Signed 32 bit"){
        sampleSize = sizeof(int32_t);
    } else if(m_traceType == "Real 32 bit (float)"){
        sampleSize = sizeof(float);
    } else if(m_traceType == "Real 64 bit (double)"){
        sampleSize = sizeof(double);
    } else {
        qFatal("Unexpected trace type while adding traces.");
        return 0;
    }

    if(length % (sampleSize * m_traceLength) != 0){
        qCritical("Buffer does not contain a valid amount of traces/samples");
        return 0;
    }

    size_t noOfTraces = length / (sampleSize * m_traceLength);

    if(m_traceType == "Unsigned 8 bit") {
        SICAK::UniHoTTestAddTraces<qreal, uint8_t>(*(m_contexts[classNo]), reinterpret_cast<const uint8_t *>(buffer), m_traceLength, noOfTraces, m_order);
    } else if(m_traceType == "Signed 8 bit"){
        SICAK::UniHoTTestAddTraces<qreal, int8_t>(*(m_contexts[classNo]), reinterpret_cast<const int8_t *>(buffer), m_traceLength, noOfTraces, m_order);
    } else if(m_traceType == "Unsigned 16 bit"){
        SICAK::UniHoTTestAddTraces<qreal, uint16_t>(*(m_contexts[classNo]), reinterpret_cast<const uint16_t *>(buffer), m_traceLength, noOfTraces, m_order);
    } else if(m_traceType == "Signed 16 bit"){
        SICAK::UniHoTTestAddTraces<qreal, int16_t>(*(m_contexts[classNo]), reinterpret_cast<const int16_t *>(buffer), m_traceLength, noOfTraces, m_order);
    } else if(m_traceType == "Unsigned 32 bit"){
        SICAK::UniHoTTestAddTraces<qreal, uint32_t>(*(m_contexts[classNo]), reinterpret_cast<const uint32_t *>(buffer), m_traceLength, noOfTraces, m_order);
    } else if(m_traceType == "Signed 32 bit"){
        SICAK::UniHoTTestAddTraces<qreal, int32_t>(*(m_contexts[classNo]), reinterpret_cast<const int32_t *>(buffer), m_traceLength, noOfTraces, m_order);
    } else if(m_traceType == "Real 32 bit (float)"){
        SICAK::UniHoTTestAddTraces<qreal, float>(*(m_contexts[classNo]), reinterpret_cast<const float *>(buffer), m_traceLength, noOfTraces, m_order);
    } else if(m_traceType == "Real 64 bit (double)"){
        SICAK::UniHoTTestAddTraces<qreal, double>(*(m_contexts[classNo]), reinterpret_cast<const double *>(buffer), m_traceLength, noOfTraces, m_order);
    } else {
        qFatal("Unexpected trace type while adding traces.");
        return 0;
    }

    return length;

}

size_t TTTestDevice::getTValues(uint8_t * buffer, size_t length, size_t class1, size_t class2, size_t order){

    SICAK::Matrix<qreal> results;
    SICAK::UniHoTTestComputeTValsDegs<qreal>(*(m_contexts[class1]), *(m_contexts[class2]), results, order);

    size_t maxLengthReal = length / sizeof(qreal);
    size_t lengthReal = results.cols();

    size_t lengthToCopy = qMin(maxLengthReal, lengthReal);

    std::memcpy(reinterpret_cast<void *>(buffer), results.data(), lengthToCopy * sizeof(qreal));

    return lengthToCopy * sizeof(qreal);

}

void TTTestDevice::resetContexts() {

    for(int i = 0; i < m_numberOfClasses; i++){
        m_contexts[i]->reset();
    }

}

