#ifndef TSCENARIOANALDEVICEWRITEITEM_H
#define TSCENARIOANALDEVICEWRITEITEM_H

#include "tscenarioanaldeviceitem.h"

/*!
 * \brief The TScenarioAnalDeviceWriteItem class represents a block that writes to selected Analytic Device.
 *
 * The class represents a block that writes to selected Analytic Device.
 * It is based on TScenarioAnalDeviceItem and has (besides the flow ports) one input port for the data to write.
 * Success or error of the write operation is indicated selecting one of the output flow ports.
 */
class TScenarioAnalDeviceWriteItem : public TScenarioAnalDeviceItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioAnalDeviceWriteItem;
    }

    TScenarioAnalDeviceWriteItem() : TScenarioAnalDeviceItem(tr("Analytic Device: write"), tr("This block writes to selected Analytic Device.")) {
        addDataInputPort("dataIn", "data", tr("Byte array with data to write."));

        TConfigParam streamParam("Input stream", "", TConfigParam::TType::TEnum, tr("Select the stream to write to."), false);
        m_params.addSubParam(streamParam);
    }

    TScenarioItem * copy() const override {
        return new TScenarioAnalDeviceWriteItem(*this);
    }

    bool validateParamsStructure(TConfigParam params) override {
        if(!TScenarioAnalDeviceItem::validateParamsStructure(params)) {
            return false;
        }

        bool iok;
        params.getSubParamByName("Input stream", &iok);

        return iok;
    }

    void updateParams(bool paramValuesChanged) override {
        TScenarioAnalDeviceItem::updateParams(paramValuesChanged);

        TAnalDeviceModel * deviceModel = getDeviceModel();

        if(deviceModel && paramValuesChanged) {
            TConfigParam * streamParam = m_params.getSubParamByName("Input stream");
            streamParam->clearEnumValues();
            streamParam->resetState();

            for(TAnalStreamSenderModel * streamModel : deviceModel->senderModels()) {
                streamParam->addEnumValue(streamModel->name());
            }
        }
    }

    TAnalStreamSenderModel * getAnalDeviceStreamSenderModel() {
        TConfigParam * streamParam = m_params.getSubParamByName("Input stream");

        if(!m_deviceModel) {
            return nullptr;
        }

        for(TAnalStreamSenderModel * streamModel : m_deviceModel->senderModels()) {
            if(streamModel->name() == streamParam->getValue()) {
                return streamModel;
            }
        }

        return nullptr;
    }

    void dataWritten(QByteArray data) {
        disconnect(m_analStreamModel, nullptr, this, nullptr);

        log(QString(tr("[%1] Written %2 bytes to %3 stream."))
            .arg(m_deviceModel->name())
            .arg(data.size())
            .arg(m_analStreamModel->name())
        );

        emit executionFinished();
    }

    void writeFailed() {
        setState(TState::TRuntimeError, tr("Write failed."));
        disconnect(m_analStreamModel, nullptr, this, nullptr);

        log(QString("[%1] Write failed.").arg(m_deviceModel->name()));
        emit executionFinished();
    }

    void writeBusy() {
        setState(TState::TRuntimeError, tr("Write failed - device busy."));
        disconnect(m_analStreamModel, nullptr, this, nullptr);

        log(QString("[%1] Write failed - device busy.").arg(m_deviceModel->name()));
        emit executionFinished();
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        size_t dataLen = inputData.value(getItemPortByName("dataIn")).length();
        if(dataLen == 0) {
            setState(TState::TRuntimeWarning, "Requested to write 0 bytes.");
            emit executionFinished();
            return;
        }

        m_analStreamModel = getAnalDeviceStreamSenderModel();

        if(!m_analStreamModel) {
            setState(TState::TRuntimeError, tr("The input stream was not found."));
            emit executionFinished();
            return;
        }

        log(QString(tr("[%1] Writing %2 bytes to %3 stream..."))
            .arg(m_deviceModel->name())
            .arg(dataLen)
            .arg(m_analStreamModel->name())
        );

        connect(m_analStreamModel, &TSenderModel::dataWritten, this, &TScenarioAnalDeviceWriteItem::dataWritten);
        connect(m_analStreamModel, &TSenderModel::writeFailed, this, &TScenarioAnalDeviceWriteItem::writeFailed);
        connect(m_analStreamModel, &TSenderModel::writeBusy, this, &TScenarioAnalDeviceWriteItem::writeBusy);

        m_analStreamModel->writeData(inputData.value(getItemPortByName("dataIn")));
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioAnalDeviceItem::setParams(params);

        m_title = "";
        m_subtitle = "no Analytic Device selected";
        if(m_params.getState(true) != TConfigParam::TState::TError) {
            m_title = m_params.getSubParamByName("Analytic Device")->getValue() + ": write";
            m_subtitle = m_params.getSubParamByName("Input stream")->getValue();
        }

        emit appearanceChanged();
        return paramsToReturn;
    }

protected:
    TAnalStreamSenderModel * m_analStreamModel = nullptr;
};

#endif // TSCENARIOANALDEVICEWRITEITEM_H
