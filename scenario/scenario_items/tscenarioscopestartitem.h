#ifndef TSCENARIOSCOPESTARTITEM_H
#define TSCENARIOSCOPESTARTITEM_H

#include "../tscenarioitem.h"
#include "tscenarioscopeitem.h"

class TScenarioScopeStartItem : public TScenarioScopeItem {

public:
    enum { TItemClass = 62 };

    TScenarioScopeStartItem() :
        TScenarioScopeItem(
              tr("Oscilloscope: start measuring"),
              tr("This block starts measurement for the selected Oscilloscope.")
        ) { }

    TScenarioItem * copy() const override {
        return new TScenarioScopeStartItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/oscilloscope.png";
    }

    void execute(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        ((TScenarioScopeItem *)this)->execute(inputData);
        m_scopeModel->runSingle();
    }

    void tracesDownloaded(size_t traces, size_t samples, TScope::TSampleType type, QList<quint8 *> buffers, bool overvoltage) {
        m_outputData.clear();
        m_outputData.insert(getItemPortByName("traceCount"), QByteArray::number(traces));
        m_outputData.insert(getItemPortByName("sampleCount"), QByteArray::number(samples));

        int sampleSize;
        QString typeName;
        switch (type) {
            case TScope::TSampleType::TUInt8:   sampleSize = 1; typeName = "UInt8";     break;
            case TScope::TSampleType::TInt8:    sampleSize = 1; typeName = "Int8";      break;
            case TScope::TSampleType::TUInt16:  sampleSize = 2; typeName = "UInt16";    break;
            case TScope::TSampleType::TInt16:   sampleSize = 2; typeName = "Int16";     break;
            case TScope::TSampleType::TUInt32:  sampleSize = 4; typeName = "UInt32";    break;
            case TScope::TSampleType::TInt32:   sampleSize = 4; typeName = "Int32";     break;
            case TScope::TSampleType::TReal32:  sampleSize = 4; typeName = "Real32";    break;
            case TScope::TSampleType::TReal64:  sampleSize = 8; typeName = "Real64";    break;
            default: break;
        }
        m_outputData.insert(getItemPortByName("type"), typeName.toUtf8());

        QByteArray buffersData;
        for(int i = 0; i < traces; i++) {
            buffersData.append((const char *)buffers[i], samples*sampleSize);
        }
        m_outputData.insert(getItemPortByName("buffers"), buffersData);

        m_outputData.insert(getItemPortByName("overvoltage"), QByteArray::number(overvoltage ? 1 : 0));
    }

    void runFailed() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - run failed")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
        emit executionFinished();
    }

    void stopFailed() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - stop failed")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
        emit executionFinished();
    }

    void downloadFailed() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - download failed")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName  = "flowOutError";
        emit executionFinished();
    }

    void tracesEmpty() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - traces empty")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
        emit executionFinished();
    }

    void stopped() {
        disconnect(m_scopeModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Trace data downloaded")).arg(m_scopeModel->name()));
        m_preferredOutputFlowPortName = "flowOut";
        emit executionFinishedWithOutput(m_outputData);
    }


};

#endif // TSCENARIOSCOPESTARTITEM_H
