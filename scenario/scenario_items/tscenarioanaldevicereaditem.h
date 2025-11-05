#ifndef TSCENARIOANALDEVICEREADITEM_H
#define TSCENARIOANALDEVICEREADITEM_H


#include "tscenarioanaldeviceitem.h"

/*!
 * \brief The TScenarioAnalDeviceReadItem class represents a block that reads from selected Analytic Device.
 *
 * The class represents a block that reads from selected Analytic Device.
 * It is based on TScenarioAnalDeviceItem and has (besides the flow ports) one input port for the number of bytes to read and one output port for the read data.
 * Success or error of the write operation is indicated selecting one of the output flow ports.
 *
 */
class TScenarioAnalDeviceReadItem : public TScenarioAnalDeviceItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioAnalDeviceReadItem;
    }

    TScenarioAnalDeviceReadItem() : TScenarioAnalDeviceItem(tr("Analytic Device: read"), tr("This block reads from selected Analytic Device.")) {
        addDataInputPort("lengthIn", "length", tr("Number of bytes to read, as integer."));
        addDataOutputPort("dataOut", "data", tr("Byte array with read data."));

        TConfigParam streamParam("Output stream", "", TConfigParam::TType::TEnum, tr("Select the stream to read from."), false);
        m_params.addSubParam(streamParam);
    }

    TScenarioItem * copy() const override {
        return new TScenarioAnalDeviceReadItem(*this);
    }

    bool validateParamsStructure(TConfigParam params) override {
        if(!TScenarioAnalDeviceItem::validateParamsStructure(params)) {
            return false;
        }

        bool iok;
        params.getSubParamByName("Output stream", &iok);

        return iok;
    }

    void updateParams(bool paramValuesChanged) override {
        TScenarioAnalDeviceItem::updateParams(paramValuesChanged);

        TAnalDeviceModel * deviceModel = getDeviceModel();

        if(deviceModel && paramValuesChanged) {
            TConfigParam * streamParam = m_params.getSubParamByName("Output stream");
            streamParam->clearEnumValues();
            streamParam->resetState();

            for(TReceiverModel * streamModel : deviceModel->receiverModels()) {
                streamParam->addEnumValue(streamModel->name());
            }
        }
    }

    TReceiverModel * getAnalDeviceStreamReceiverModel() {
        TConfigParam * streamParam = m_params.getSubParamByName("Output stream");

        if(!m_deviceModel) {
            return nullptr;
        }

        for(TReceiverModel * streamModel : m_deviceModel->receiverModels()) {
            if(streamModel->name() == streamParam->getValue()) {
                return streamModel;
            }
        }

        return nullptr;
    }

    void dataRead(QByteArray data) {
        disconnect(m_analStreamModel, nullptr, this, nullptr);

        QHash<TScenarioItemPort *, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), data);

        log(QString(tr("[%1] Read %2 bytes from %3 stream."))
            .arg(m_deviceModel->name())
            .arg(data.size())
            .arg(m_analStreamModel->name())
        );

        emit executionFinished(outputData);
    }

    void readFailed() {
        setState(TState::TRuntimeError, tr("Read failed."));
        disconnect(m_analStreamModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed.")).arg(m_deviceModel->name()));
        emit executionFinished();
    }

    void readBusy() {
        setState(TState::TRuntimeError, tr("Read failed - device busy."));
        disconnect(m_analStreamModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - device busy.")).arg(m_deviceModel->name()));
        emit executionFinished();
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        int dataLen;
        QDataStream lengthStream(inputData.value(getItemPortByName("lengthIn")));
        lengthStream >> dataLen;

        if(dataLen == 0) {
            setState(TState::TRuntimeWarning, tr("Requested to read 0 bytes."));
            emit executionFinished();
            return;
        }

        m_analStreamModel = getAnalDeviceStreamReceiverModel();

        if(!m_analStreamModel) {
            setState(TState::TRuntimeError, tr("The output stream was not found."));
            emit executionFinished();
            return;
        }

        log(QString(tr("[%1] Reading %2 bytes from %3 stream..."))
            .arg(m_deviceModel->name())
            .arg(dataLen)
            .arg(m_analStreamModel->name())
        );

        connect(m_analStreamModel, &TReceiverModel::dataRead, this, &TScenarioAnalDeviceReadItem::dataRead);
        connect(m_analStreamModel, &TReceiverModel::readFailed, this, &TScenarioAnalDeviceReadItem::readFailed);
        connect(m_analStreamModel, &TReceiverModel::readBusy, this, &TScenarioAnalDeviceReadItem::readBusy);

        m_analStreamModel->readData(dataLen);
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioAnalDeviceItem::setParams(params);

        m_title = "";
        m_subtitle = tr("no Analytic Device selected");
        if(m_params.getState(true) != TConfigParam::TState::TError) {
            m_title = m_params.getSubParamByName("Analytic Device")->getValue() + ": read";
            m_subtitle = m_params.getSubParamByName("Output stream")->getValue();
        }

        emit appearanceChanged();
        return paramsToReturn;
    }

protected:
    TReceiverModel * m_analStreamModel = nullptr;
};

#endif // TSCENARIOANALDEVICEREADITEM_H
