#ifndef TSCENARIOIODEVICEWRITEITEM_H
#define TSCENARIOIODEVICEWRITEITEM_H

#include "tscenarioiodeviceitem.h"

/*!
 * \brief The TScenarioIODeviceWriteItem class represents a block that writes to selected IO Device.
 *
 * The class represents a block that writes to selected IO Device.
 * It is based on TScenarioIODeviceItem and has (besides the flow ports) one input port for the data to write.
 * Success or error of the write operation is indicated selecting one of the output flow ports.
 */
class TScenarioIODeviceWriteItem : public TScenarioIODeviceItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioIODeviceWriteItem;
    }

    TScenarioIODeviceWriteItem() : TScenarioIODeviceItem(tr("IO Device: write"), tr("This block writes to selected IO Device.")) {
        addDataInputPort("dataIn", "data", tr("Byte array with data to write."), "[byte array]");
    }

    TScenarioItem * copy() const override {
        return new TScenarioIODeviceWriteItem(*this);
    }

    void dataWritten(QByteArray data) {
        disconnect(m_deviceModel->senderModel(), nullptr, this, nullptr);

        log(QString("[%1] Written %2 bytes.").arg(m_deviceModel->name()).arg(data.size()));
        emit executionFinished();
    }

    void writeFailed() {
        setState(TState::TRuntimeError, tr("Write failed."));
        disconnect(m_deviceModel->senderModel(), nullptr, this, nullptr);

        log(QString("[%1] Write failed.").arg(m_deviceModel->name()));
        emit executionFinished();
    }

    void writeBusy() {
        setState(TState::TRuntimeError, tr("Write failed - device busy."));
        disconnect(m_deviceModel->senderModel(), nullptr, this, nullptr);

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

        if(!m_deviceModel || !m_deviceModel->receiverModel()) {
            setState(TState::TRuntimeError, tr("The target device is not initialized or was not found."));
            emit executionFinished();
            return;
        }

        log(QString("[%1] Writing %2 bytes...").arg(m_deviceModel->name()).arg(dataLen));

        connect(m_deviceModel->senderModel(), &TSenderModel::dataWritten, this, &TScenarioIODeviceWriteItem::dataWritten);
        connect(m_deviceModel->senderModel(), &TSenderModel::writeFailed, this, &TScenarioIODeviceWriteItem::writeFailed);
        connect(m_deviceModel->senderModel(), &TSenderModel::writeBusy, this, &TScenarioIODeviceWriteItem::writeBusy);

        m_deviceModel->senderModel()->writeData(inputData.value(getItemPortByName("dataIn")));
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioIODeviceItem::setParams(params);

        m_title = "";
        m_subtitle = "no IO device selected";
        if(m_params.getState(true) != TConfigParam::TState::TError) {
            m_title = m_params.getSubParamByName("Component")->getValue() + ": write";
            m_subtitle = m_params.getSubParamByName("IO Device")->getValue();
        }

        emit appearanceChanged();
        return paramsToReturn;
    }
};

#endif // TSCENARIOIODEVICEWRITEITEM_H
