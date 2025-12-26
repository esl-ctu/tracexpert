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

#ifndef TSCENARIOSCOPEITEM_H
#define TSCENARIOSCOPEITEM_H

#include "tscenariocomponentitem.h"

/**
 * @brief The TScenarioScopeItem class represents a block that interfaces a selected Scope.
 *
 * The class represents a block that interfaces a selected Scope.
 * It is a block with one input flow port and output flow ports for the number of traces,
 * number of samples per trace, data type of samples, sample data and overvoltage indication.
 *
 * The Scope can be selected in the configuration. It is selected from the list of available Scopes in the selected Component.
 * The Scope can be configured with pre-init and post-init parameters that can be set once,
 * before the scenario is launched, before the block is first executed, or each time the block is executed.
 *
 */
class TScenarioScopeItem : public TScenarioComponentItem<TScopeModel> {

public:
    TScenarioScopeItem(QString name, QString description) : TScenarioComponentItem<TScopeModel>("Oscilloscope", name, description) { }

    void initializeDataOutputPorts() {
        m_channelPortCount = 1;
        addDataOutputPort("traceCount", "#traces",tr("Number of traces."), "[unsigned long long]");
        addDataOutputPort("sampleCount", "#samples", tr("Number of samples per trace."), "[unsigned long long]");
        addDataOutputPort("type", "data type", tr("Data type of samples."), "[string]");
        addDataOutputPort("overvoltage", "overvoltage", tr("Overvoltage indication."), "[bool]");
        addDataOutputPort("channel1", "ch. 1 data", tr("Byte array with data from the channel."), "[byte array]");
    }

    TScenarioItem * copy() const override {
        return new TScenarioScopeItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/oscilloscope.png";
    }

    bool isCompatibleComponent(TComponentModel * componentModel) override {
        return componentModel->canAddScope() || componentModel->scopeCount() > 0;
    }

    int deviceCount(TComponentModel * componentModel) override {
        return componentModel->scopeCount();
    }

    TScopeModel * deviceByIndex(TComponentModel * componentModel, int index) override {
        return componentModel->scope(index);
    }

    TScopeModel * deviceByName(TComponentModel * componentModel, const QString & name) override {
        return componentModel->scopeContainer()->getByName(name);
    }

    void drawChannelOutputPorts(TScopeModel * scopeModel) {
        quint32 channelCount = m_channelPortCount;

        if(scopeModel) {
            channelCount = scopeModel->channelsStatus().count();
        }

        if(channelCount != m_channelPortCount) {
            for(quint32 i = 0; i < std::max(channelCount, m_channelPortCount); i++) {
                if(i >= channelCount) {
                    // remove this port
                    removePort(QString("channel%1").arg(i+1));
                }
                else {
                    // create a new port
                    addDataOutputPort(
                        QString("channel%1").arg(i+1),
                        QString("ch. %1 data").arg(i+1),
                        tr("Byte array with data from the channel."),
                        "[byte array]"
                    );
                }
            }

            m_channelPortCount = channelCount;
        }
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioComponentItem::setParams(params);

        m_subtitle = "no Oscilloscope selected";
        if(m_params.getState(true) != TConfigParam::TState::TError) {
            m_subtitle = m_params.getSubParamByName("Oscilloscope")->getValue();
        }

        emit appearanceChanged();
        return paramsToReturn;
    }

    bool cleanup() override {
        TScenarioComponentItem::cleanup();
        m_outputData.clear();
        return true;
    }

    void tracesDownloaded(size_t traces, size_t samples, TScope::TSampleType type, QList<QByteArray> buffers, bool overvoltage) {
        log(tr("[%1] Trace data downloaded").arg(m_deviceModel->name()));

        QList<TScope::TChannelStatus> status = m_deviceModel->channelsStatus();

        if(buffers.count() != status.count()) {
            log(tr("[%1] Incorrect number of data buffers received").arg(m_deviceModel->name()), TLogLevel::TError);
            return;
        }

        m_outputData.clear();
        m_outputData.insert(getItemPortByName("traceCount"), QByteArray::number(traces));
        m_outputData.insert(getItemPortByName("sampleCount"), QByteArray::number(samples));

        QString typeName;
        switch (type) {
            case TScope::TSampleType::TUInt8:   typeName = "UInt8";     break;
            case TScope::TSampleType::TInt8:    typeName = "Int8";      break;
            case TScope::TSampleType::TUInt16:  typeName = "UInt16";    break;
            case TScope::TSampleType::TInt16:   typeName = "Int16";     break;
            case TScope::TSampleType::TUInt32:  typeName = "UInt32";    break;
            case TScope::TSampleType::TInt32:   typeName = "Int32";     break;
            case TScope::TSampleType::TReal32:  typeName = "Real32";    break;
            case TScope::TSampleType::TReal64:  typeName = "Real64";    break;
            default: break;
        }
        m_outputData.insert(getItemPortByName("type"), typeName.toUtf8());

        for (int i = 0; i < status.count(); i++) {
            TScenarioItemPort * channelPort = getItemPortByName(QString("channel%1").arg(i+1));

            if(!channelPort) {
                log(tr("[%1] Channel %2 port unavailable.").arg(m_deviceModel->name()).arg(i+1), TLogLevel::TError);
                return;
            }

            m_outputData.insert(channelPort, buffers[i]);

            // assuming no other part of the program will be using the data
            // and to free space for the data array (since we might be dealing with very big data)
            // let's delete the oroginal buffer
            buffers[i].clear();
        }

        m_outputData.insert(getItemPortByName("overvoltage"), QByteArray::number(overvoltage ? 1 : 0));
    }

    void clearOutputData() {
        m_outputData.clear();
    }

    virtual void runFailed() {
        disconnect(m_deviceModel, nullptr, this, nullptr);

        log(tr("[%1] Read failed - run failed").arg(m_deviceModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
    }

    virtual void stopFailed() {
        disconnect(m_deviceModel, nullptr, this, nullptr);

        log(tr("[%1] Read failed - stop failed").arg(m_deviceModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
    }

    virtual void downloadFailed() {
        disconnect(m_deviceModel, nullptr, this, nullptr);

        log(tr("[%1] Read failed - download failed").arg(m_deviceModel->name()));
        m_preferredOutputFlowPortName  = "flowOutError";
    }

    virtual void tracesEmpty() {
        disconnect(m_deviceModel, nullptr, this, nullptr);

        log(tr("[%1] Read failed - traces empty").arg(m_deviceModel->name()));
        m_preferredOutputFlowPortName = "flowOutError";
    }

    virtual void stopped() {
        disconnect(m_deviceModel, nullptr, this, nullptr);

        log(tr("[%1] Measurement stopped").arg(m_deviceModel->name()));
        m_preferredOutputFlowPortName = "flowOut";
    }


protected:
    QHash<TScenarioItemPort *, QByteArray> m_outputData;
    quint32 m_channelPortCount = 0;
};

#endif // TSCENARIOSCOPEITEM_H
