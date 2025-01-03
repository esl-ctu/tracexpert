#include "tttestdevice.h"

#include <QtGlobal>

#include "tttestaction.h"
#include "tttestinputstream.h"
#include "tttestoutputstream.h"
#include "ttest.hpp"

TTTestDevice::TTTestDevice() {
    m_preInitParams = TConfigParam("Welch's t-test configuration", "", TConfigParam::TType::TDummy, "");
    TConfigParam traceLength = TConfigParam("Trace length (in samples)", "1000", TConfigParam::TType::TUInt, "The number of samples per data trace");
    m_preInitParams.addSubParam(traceLength);
    TConfigParam numberOfClasses = TConfigParam("Number of classes", "2", TConfigParam::TType::TUInt, "The number of classes to perform t-test between");
    m_preInitParams.addSubParam(numberOfClasses);
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
    TConfigParam order = TConfigParam("Maximum order", "1", TConfigParam::TType::TUInt, "Maximum order of the t-test");
    m_preInitParams.addSubParam(order);

    TConfigParam inputFormat = TConfigParam("Input format", "Input stream per class", TConfigParam::TType::TEnum, "Input format. Labels must be numbers of the class (starting with 0).");
    inputFormat.addEnumValue("Input stream per class");
    inputFormat.addEnumValue("Label + traces streams");
    TConfigParam labelType = TConfigParam("Label data type", "Unsigned 8 bit", TConfigParam::TType::TEnum, "Data type of the samples");
    labelType.addEnumValue("Unsigned 8 bit");
    labelType.addEnumValue("Signed 8 bit");
    labelType.addEnumValue("Unsigned 16 bit");
    labelType.addEnumValue("Signed 16 bit");
    labelType.addEnumValue("Unsigned 32 bit");
    labelType.addEnumValue("Signed 32 bit");
    inputFormat.addSubParam(labelType);
    m_preInitParams.addSubParam(inputFormat);

}

TTTestDevice::~TTTestDevice() {
    (*this).TTTestDevice::deInit();
}

QString TTTestDevice::getName() const {
    return QString("Welch's t-test");
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
        traceLengthParam->setState(TConfigParam::TState::TError, "Trace length must be a positive integer");
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

    TConfigParam * traceTypeParam = m_preInitParams.getSubParamByName("Sample data type", &iok);
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

    TConfigParam * inputFormatParam = m_preInitParams.getSubParamByName("Input format", &iok);
    if(!iok) {
        qCritical("Input format parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    QString inputFormatVal = inputFormatParam->getValue();
    if(inputFormatVal == "Input stream per class") {
        m_inputFormat = 0;
    } else if(inputFormatVal == "Label + traces streams"){
        m_inputFormat = 1;
    } else {
        inputFormatParam->setState(TConfigParam::TState::TError, "Invalid input format parameter.");
    }

    TConfigParam * labelTypeParam = inputFormatParam->getSubParamByName("Label data type", &iok);
    if(!iok) {
        qCritical("Label format parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    m_labelType = labelTypeParam->getValue();

    return m_preInitParams;
}

void TTTestDevice::init(bool *ok) {

    m_analActions.append(new TTTestAction("Compute t-values", "Computes t-values", [=](){ computeTVals(); }));
    m_analActions.append(new TTTestAction("Reset (delete all data)", "Deletes all the received data", [=](){ resetContexts(); }));

    if(m_inputFormat == 1){ // 1 - labels

        m_analOutputStreams.append(new TTTestOutputStream("Labels", "Stream of labels", [=](const uint8_t * buffer, size_t length){ return addLabels(buffer, length); }));
        m_analOutputStreams.append(new TTTestOutputStream("Traces", "Stream of traces", [=](const uint8_t * buffer, size_t length){ return addLabeledTraces(buffer, length); }));

    } else { // 0 - a stream per class

        for(int i = 0; i < m_numberOfClasses; i++){
            QString streamName = QString("Class %1").arg(i);
            m_analOutputStreams.append(new TTTestOutputStream(streamName, "Stream of traces in respective class", [=, classNo = i](const uint8_t * buffer, size_t length){ return addTraces(buffer, length, classNo); }));
        }

    }

    for(int order = 1; order <= m_order; order++){
        for (int i = 0; i < m_numberOfClasses; i++) {
            for (int j = i + 1; j < m_numberOfClasses; j++) {

                QString streamName = QString("%1-order t-vals %2 vs %3").arg(order).arg(i).arg(j);
                m_analInputStreams.append(new TTTestInputStream(streamName, "Stream of t-values", [=, class1 = i, class2 = j, order0 = order](uint8_t * buffer, size_t length){ return getTValues(buffer, length, class1, class2, order0); }));

                m_tvals.append(new SICAK::Matrix<qreal>());
                m_position.append(0);

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

    for (int i = 0; i < m_tvals.length(); i++) {
        delete m_tvals[i];
    }
    m_tvals.clear();

    m_position.clear();

    while(!m_labeledTraces.empty()){
        SICAK::Vector<uint8_t> * trace = m_labeledTraces.dequeue();
        delete trace;
    }

    m_labels.clear();

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

    size_t sampleSize = getTypeSize(m_traceType);

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

void TTTestDevice::resetContexts() {

    for(int i = 0; i < m_numberOfClasses; i++){
        m_contexts[i]->reset();
    }

    while(!m_labeledTraces.empty()){
        SICAK::Vector<uint8_t> * trace = m_labeledTraces.dequeue();
        delete trace;
    }

    m_labels.clear();

    for (int i = 0; i < m_tvals.length(); i++) {
        m_tvals[i]->fill(0);
    }

    for (int i = 0; i < m_position.length(); i++) {
        m_position[i] = m_tvals[i]->cols()*sizeof(qreal);
    }

}

size_t TTTestDevice::addLabeledTraces(const uint8_t * buffer, size_t length){

    size_t sampleSize = getTypeSize(m_traceType);

    if(length % (sampleSize * m_traceLength) != 0){
        qCritical("Buffer does not contain a valid amount of traces/samples");
        return 0;
    }

    size_t noOfTraces = length / (sampleSize * m_traceLength);

    for(int i = 0; i < noOfTraces; i++){
        SICAK::Vector<uint8_t> * trace = new SICAK::Vector<uint8_t>(sampleSize * m_traceLength);
        std::memcpy(trace->data(), &(buffer[i * sampleSize * m_traceLength]), sampleSize * m_traceLength);

        m_labeledTraces.enqueue(trace);
    }

    this->processLabeledTraces();

    return length;
}

size_t TTTestDevice::addLabels(const uint8_t * buffer, size_t length){

    size_t labelSize = getTypeSize(m_labelType);

    if(length % labelSize != 0){
        qCritical("Buffer does not contain a valid amount of labels with respect to the data type size");
        return 0;
    }

    size_t noOfLabels = length / labelSize;

    for(int i = 0; i < noOfLabels; i++){

        if(m_labelType == "Unsigned 8 bit") {
            m_labels.enqueue((reinterpret_cast<const uint8_t *>(buffer))[i]);
        } else if(m_labelType == "Signed 8 bit"){
            m_labels.enqueue((reinterpret_cast<const int8_t *>(buffer))[i]);
        } else if(m_labelType == "Unsigned 16 bit"){
            m_labels.enqueue((reinterpret_cast<const uint16_t *>(buffer))[i]);
        } else if(m_labelType == "Signed 16 bit"){
            m_labels.enqueue((reinterpret_cast<const int16_t *>(buffer))[i]);
        } else if(m_labelType == "Unsigned 32 bit"){
            m_labels.enqueue((reinterpret_cast<const uint32_t *>(buffer))[i]);
        } else if(m_labelType == "Signed 32 bit"){
            m_labels.enqueue((reinterpret_cast<const int32_t *>(buffer))[i]);
        } else if(m_labelType == "Real 32 bit (float)"){
            m_labels.enqueue((reinterpret_cast<const float *>(buffer))[i]);
        } else if(m_labelType == "Real 64 bit (double)"){
            m_labels.enqueue((reinterpret_cast<const double *>(buffer))[i]);
        } else {
            qFatal("Unexpected trace type while adding traces.");
            return 0;
        }

    }

    this->processLabeledTraces();

    return length;
}

size_t TTTestDevice::getTypeSize(const QString & dataType){

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

void TTTestDevice::processLabeledTraces(){

    while((!m_labeledTraces.empty()) && (!m_labels.empty())){

        size_t label = m_labels.dequeue();
        SICAK::Vector<uint8_t> * trace = m_labeledTraces.dequeue();

        if(label >= m_numberOfClasses){
            qCritical("Label of the trace is out of range");
            continue;
        }

        if(m_traceType == "Unsigned 8 bit") {
            SICAK::UniHoTTestAddTraces<qreal, uint8_t>(*(m_contexts[label]), reinterpret_cast<const uint8_t *>(trace->data()), m_traceLength, 1, m_order);
        } else if(m_traceType == "Signed 8 bit"){
            SICAK::UniHoTTestAddTraces<qreal, int8_t>(*(m_contexts[label]), reinterpret_cast<const int8_t *>(trace->data()), m_traceLength, 1, m_order);
        } else if(m_traceType == "Unsigned 16 bit"){
            SICAK::UniHoTTestAddTraces<qreal, uint16_t>(*(m_contexts[label]), reinterpret_cast<const uint16_t *>(trace->data()), m_traceLength, 1, m_order);
        } else if(m_traceType == "Signed 16 bit"){
            SICAK::UniHoTTestAddTraces<qreal, int16_t>(*(m_contexts[label]), reinterpret_cast<const int16_t *>(trace->data()), m_traceLength, 1, m_order);
        } else if(m_traceType == "Unsigned 32 bit"){
            SICAK::UniHoTTestAddTraces<qreal, uint32_t>(*(m_contexts[label]), reinterpret_cast<const uint32_t *>(trace->data()), m_traceLength, 1, m_order);
        } else if(m_traceType == "Signed 32 bit"){
            SICAK::UniHoTTestAddTraces<qreal, int32_t>(*(m_contexts[label]), reinterpret_cast<const int32_t *>(trace->data()), m_traceLength, 1, m_order);
        } else if(m_traceType == "Real 32 bit (float)"){
            SICAK::UniHoTTestAddTraces<qreal, float>(*(m_contexts[label]), reinterpret_cast<const float *>(trace->data()), m_traceLength, 1, m_order);
        } else if(m_traceType == "Real 64 bit (double)"){
            SICAK::UniHoTTestAddTraces<qreal, double>(*(m_contexts[label]), reinterpret_cast<const double *>(trace->data()), m_traceLength, 1, m_order);
        } else {
            qFatal("Unexpected trace type while adding traces.");
        }

        delete trace;

    }

}

void TTTestDevice::computeTVals(){

    int k = 0;

    for(int order = 1; order <= m_order; order++){
        for (int i = 0; i < m_numberOfClasses; i++) {
            for (int j = i + 1; j < m_numberOfClasses; j++) {

                SICAK::UniHoTTestComputeTValsDegs<qreal>(*(m_contexts[i]), *(m_contexts[j]), *(m_tvals[k]), order);
                m_position[k] = 0;
                k++;

            }
        }
    }

}

size_t TTTestDevice::getTValues(uint8_t * buffer, size_t length, size_t class1, size_t class2, size_t order){

    int k = 0;

    for(int orderIdx = 1; orderIdx <= m_order; order++){
        for (int i = 0; i < m_numberOfClasses; i++) {
            for (int j = i + 1; j < m_numberOfClasses; j++) {

                if(orderIdx == order && i == class1 && j == class2) {

                    int sent = 0;

                    for (sent = 0; m_position[k] < (m_tvals[k]->cols() * sizeof(qreal)) && sent < length; sent++, m_position[k]++) {
                        buffer[sent] = *(reinterpret_cast<uint8_t *>(m_tvals[k]->data()) + m_position[k]);
                    }

                    return sent;

                }

                k++;

            }
        }
    }

    return 0;

}
