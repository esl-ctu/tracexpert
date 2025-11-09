#include "tcpadevice.h"

#include <QtGlobal>

#include "tcpaaction.h"
#include "tcpainputstream.h"
#include "tcpaoutputstream.h"
#include "cpa.hpp"

TCPADevice::TCPADevice(): m_traceLength(0), m_predictCount(0), m_traceType("Unsigned 8 bit"), m_predictType("Unsigned 8 bit"), m_order(1) {

    m_preInitParams = TConfigParam("CPA configuration", "", TConfigParam::TType::TDummy, "");

    TConfigParam traceLength = TConfigParam("Trace length (in samples)", "1000", TConfigParam::TType::TUInt, "The number of samples per trace");
    m_preInitParams.addSubParam(traceLength);
    TConfigParam traceType = TConfigParam("Sample data type", "Unsigned 8 bit", TConfigParam::TType::TEnum, "Data type of the samples");
    traceType.addEnumValue("Unsigned 8 bit");
    traceType.addEnumValue("Signed 8 bit");
    traceType.addEnumValue("Unsigned 16 bit");
    traceType.addEnumValue("Signed 16 bit");
    traceType.addEnumValue("Unsigned 32 bit");
    traceType.addEnumValue("Signed 32 bit");
    traceType.addEnumValue("Real 32 bit (float)");
    traceType.addEnumValue("Real 64 bit (double)");
    m_preInitParams.addSubParam(traceType);

    TConfigParam numberOfClasses = TConfigParam("Number of predictions per trace", "256", TConfigParam::TType::TUInt, "The number power predictions/key candidates");
    m_preInitParams.addSubParam(numberOfClasses);
    TConfigParam predictionType = TConfigParam("Prediction data type", "Unsigned 8 bit", TConfigParam::TType::TEnum, "Data type of the samples");
    predictionType.addEnumValue("Unsigned 8 bit");
    predictionType.addEnumValue("Signed 8 bit");
    predictionType.addEnumValue("Unsigned 16 bit");
    predictionType.addEnumValue("Signed 16 bit");
    predictionType.addEnumValue("Unsigned 32 bit");
    predictionType.addEnumValue("Signed 32 bit");
    predictionType.addEnumValue("Real 32 bit (float)");
    predictionType.addEnumValue("Real 64 bit (double)");
    m_preInitParams.addSubParam(predictionType);

    TConfigParam order = TConfigParam("Maximum order", "1", TConfigParam::TType::TUInt, "Maximum order of the CPA");
    m_preInitParams.addSubParam(order);

}

TCPADevice::~TCPADevice() {
    (*this).TCPADevice::deInit();
}

QString TCPADevice::getName() const {
    return QString("Univariate CPA");
}

QString TCPADevice::getInfo() const {
    return QString("Default device for a CPA attack");
}


TConfigParam TCPADevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TCPADevice::setPreInitParams(TConfigParam params) {

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
        traceLengthParam->setState(TConfigParam::TState::TError, "Trace length must be a positive integer");
    }        

    TConfigParam * predictCountParam = m_preInitParams.getSubParamByName("Number of predictions per trace", &iok);
    if(!iok) {
        qCritical("Number of predictions parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_predictCount = predictCountParam->getValue().toUInt(&iok);
    if (!iok || m_predictCount < 2) {
        predictCountParam->setState(TConfigParam::TState::TError, "Number of predictions must be 2 or greater.");
    }        

    TConfigParam * traceTypeParam = m_preInitParams.getSubParamByName("Sample data type", &iok);
    if(!iok) {
        qCritical("Trace data type parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_traceType = traceTypeParam->getValue();

    TConfigParam * predictTypeParam = m_preInitParams.getSubParamByName("Prediction data type", &iok);
    if(!iok) {
        qCritical("Prediction data type parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_predictType = predictTypeParam->getValue();

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

void TCPADevice::init(bool *ok) {

    m_analActions.append(new TCPAAction("Compute correlation matrix (+ flush streams)", "", [=](){ computeCorrelations(); }));
    m_analActions.append(new TCPAAction("Reset (delete all data)", "", [=](){ resetContexts(); }));

    m_analOutputStreams.append(new TCPAOutputStream("Traces", "Stream of traces", [=](const uint8_t * buffer, size_t length){ return addTraces(buffer, length); }));
    m_analOutputStreams.append(new TCPAOutputStream("Predictions", "Stream of predictions", [=](const uint8_t * buffer, size_t length){ return addPredicts(buffer, length); }));

    for(int order = 1; order <= m_order; order++){

        QString streamName = QString("%1-order correlation matrix").arg(order);
        m_analInputStreams.append(new TCPAInputStream(streamName, "Stream of correlation coefficients", [=, order0 = order](uint8_t * buffer, size_t length){ return getCorrelations(buffer, length, order0); }, [=, order0 = order](){ return availableBytes(order0); }));

        m_correlations.append(new SICAK::Matrix<qreal>());
        m_position.append(0);

    }

    m_context = SICAK::Moments2DContext<qreal>(m_traceLength, m_predictCount, 1, 1, 2 * m_order, 2, m_order);
    m_context.reset();

    if (ok != nullptr) *ok = true;

}

void TCPADevice::deInit(bool *ok) {

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

    m_context.reset();

    m_traces.clear();

    for (int i = 0; i < m_correlations.length(); i++) {
        delete m_correlations[i];
    }
    m_correlations.clear();

    m_position.clear();

    if (ok != nullptr) *ok = true;
}

TConfigParam TCPADevice::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TCPADevice::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

QList<TAnalAction *> TCPADevice::getActions() const
{
    return m_analActions;
}

QList<TAnalInputStream *> TCPADevice::getInputDataStreams() const
{
    return m_analInputStreams;
}

QList<TAnalOutputStream *> TCPADevice::getOutputDataStreams() const
{
    return m_analOutputStreams;
}

bool TCPADevice::isBusy() const
{
    return false;
}

size_t TCPADevice::addTraces(const uint8_t * buffer, size_t length){

    m_traces.reserve(m_traces.size() + length); // preallocate space

    for(int i = 0; i < length; i++){
        m_traces.append(buffer[i]);
    }

    return length;

}

size_t TCPADevice::addPredicts(const uint8_t * buffer, size_t length){

    m_predicts.reserve(m_predicts.size() + length); // preallocate space

    for(int i = 0; i < length; i++){
        m_predicts.append(buffer[i]);
    }

    return length;

}

void TCPADevice::resetContexts() {

    m_context.reset();
    m_traces.clear();
    m_predicts.clear();

    for (int i = 0; i < m_correlations.length(); i++) {
        m_correlations[i]->fill(0);
    }

    for (int i = 0; i < m_position.length(); i++) {
        m_position[i] = m_correlations[i]->size();
    }

    qInfo("All previously submitted or computed (unread) data have been erased.");
}


size_t TCPADevice::getTypeSize(const QString & dataType){

    if(dataType == "Unsigned 8 bit") {
        return sizeof(uint8_t);
    } else if(dataType == "Signed 8 bit"){
        return sizeof(int8_t);
    } else if(dataType == "Unsigned 16 bit"){
        return sizeof(uint16_t);
    } else if(dataType == "Signed 16 bit"){
        return sizeof(int16_t);
    } else if(dataType == "Unsigned 32 bit"){
        return sizeof(uint32_t);
    } else if(dataType == "Signed 32 bit"){
        return sizeof(int32_t);
    } else if(dataType == "Real 32 bit (float)"){
        return sizeof(float);
    } else if(dataType == "Real 64 bit (double)"){
        return sizeof(double);
    } else {
        qFatal("Unexpected trace type while adding traces.");
        return 0;
    }

}

void TCPADevice::computeCorrelations(){

    if(m_traces.size() % (getTypeSize(m_traceType) * m_traceLength) != 0){
        qCritical("Buffer does not contain a valid amount of traces/samples");
        return;
    }

    if(m_predicts.size() % (getTypeSize(m_predictType) * m_predictCount) != 0){
        qCritical("Buffer does not contain a valid amount of predictions");
        return;
    }

    size_t noOfTraces = m_traces.size() / (getTypeSize(m_traceType) * m_traceLength);
    size_t noOfPredictSets = m_predicts.size() / (getTypeSize(m_predictType) * m_predictCount);

    if(noOfTraces != noOfPredictSets){
        qCritical("Number of traces and number of prediction sets does not match!");
        return;
    }

    qInfo("Unread correlation matrices were erased. The submitted data will be added to all the previously submitted data (unless the Reset action was run), and the new correlations will be computed upon all of these.");
    qInfo(QString("Now processing %1 bytes of power traces (%2 power traces, %3 samples each) and %4 bytes of leakage predictions (%2 sets, one for every trace, each consisting of %5 leakage predictions).").arg(m_traces.size()).arg(noOfTraces).arg(m_traceLength).arg(m_predicts.size()).arg(m_predictCount).toLatin1());

    // Add traces to the context
    // ... sorry (need explicit cast on both traces and predictions, shit happens)
    if(m_traceType == "Unsigned 8 bit") {

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint8_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else if(m_traceType == "Signed 8 bit"){

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int8_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else if(m_traceType == "Unsigned 16 bit"){

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint16_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else if(m_traceType == "Signed 16 bit"){

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int16_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else if(m_traceType == "Unsigned 32 bit"){

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<uint32_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else if(m_traceType == "Signed 32 bit"){

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<int32_t *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else if(m_traceType == "Real 32 bit (float)"){

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<float *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else if(m_traceType == "Real 64 bit (double)"){

        if(m_predictType == "Unsigned 8 bit") {

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<uint8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 8 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<int8_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<uint16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 16 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<int16_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Unsigned 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<uint32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Signed 32 bit"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<int32_t *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 32 bit (float)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<float *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else if(m_predictType == "Real 64 bit (double)"){

            if(m_order == 1) {

                SICAK::UniFoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength);

            } else {

                SICAK::UniHoCpaAddTraces(m_context, reinterpret_cast<double *>(m_traces.data()), reinterpret_cast<double *>(m_predicts.data()), noOfTraces, m_predictCount, m_traceLength, m_order);

            }

        } else {
            qFatal("Unexpected predictions type while adding traces.");
            return;
        }

    } else {
        qFatal("Unexpected trace type while adding traces.");
        return;
    }

    // Clear processed data from the buffers
    m_traces.clear();
    m_predicts.clear();

    // compute correlation matrices
    if(m_order == 1){

        SICAK::UniFoCpaComputeCorrelationMatrix(m_context, *(m_correlations[0]));
        m_position[0] = 0;

    } else {

        for(int i = 1; i <= m_order; i++){

            SICAK::UniHoCpaComputeCorrelationMatrix(m_context, *(m_correlations[i-1]), i);
            m_position[i-1] = 0;

        }

    }

    qInfo(QString("Generated %1 bytes of data (%2 x %3 correlation matrices) on every stream, now available for reading.").arg(m_correlations[0]->size()).arg(m_traceLength).arg(m_predictCount).toLatin1());

}

size_t TCPADevice::getCorrelations(uint8_t * buffer, size_t length, size_t order0){

    int sent = 0;

    for (sent = 0; m_position[order0-1] < (m_correlations[order0-1]->size()) && sent < length; sent++, m_position[order0-1]++) {
        buffer[sent] = *(reinterpret_cast<uint8_t *>(m_correlations[order0-1]->data()) + m_position[order0-1]);
    }

    return sent;

}

size_t TCPADevice::availableBytes(size_t order){
    return m_correlations[order-1]->size() - m_position[order-1];
}
