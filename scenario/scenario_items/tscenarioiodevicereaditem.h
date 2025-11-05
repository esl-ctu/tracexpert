#ifndef TSCENARIOIODEVICEREADITEM_H
#define TSCENARIOIODEVICEREADITEM_H


#include "tscenarioiodeviceitem.h"

/*!
 * \brief The TScenarioIODeviceReadItem class represents a block that reads from selected IO Device.
 *
 * The class represents a block that reads from selected IO Device.
 * It is based on TScenarioIODeviceItem and has (besides the flow ports) one input port for the number of bytes to read and one output port for the read data.
 * Success or error of the write operation is indicated selecting one of the output flow ports.
 *
 */
class TScenarioIODeviceReadItem : public TScenarioIODeviceItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioIODeviceReadItem;
    }

    TScenarioIODeviceReadItem() : TScenarioIODeviceItem(tr("IO Device: read"), tr("This block reads from selected IO Device.")) {
        addDataInputPort("lengthIn", "length", tr("Number of bytes to read, as integer."));
        addDataOutputPort("dataOut", "data", tr("Byte array with read data."));
    }

    TScenarioItem * copy() const override {
        return new TScenarioIODeviceReadItem(*this);
    }

    void dataRead(QByteArray data) {
        disconnect(m_deviceModel->receiverModel(), nullptr, this, nullptr);

        QHash<TScenarioItemPort *, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), data);

        log(QString(tr("[%1] Read %2 bytes.")).arg(m_deviceModel->name()).arg(data.size()));

        emit executionFinished(outputData);
    }

    void readFailed() {
        setState(TState::TRuntimeError, tr("Read failed."));
        disconnect(m_deviceModel->receiverModel(), nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed.")).arg(m_deviceModel->name()));
        emit executionFinished();
    }

    void readBusy() {
        setState(TState::TRuntimeError, tr("Read failed - device busy."));
        disconnect(m_deviceModel->receiverModel(), nullptr, this, nullptr);

        log(QString(tr("[%1] Read failed - device busy.")).arg(m_deviceModel->name()));
        emit executionFinished();
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        checkAndSetInitParamsBeforeExecution();

        int dataLen;
        QDataStream lengthStream(inputData.value(getItemPortByName("lengthIn")));
        lengthStream.setByteOrder(QDataStream::LittleEndian);
        lengthStream >> dataLen;

        if(dataLen == 0) {
            setState(TState::TRuntimeWarning, tr("Requested to read 0 bytes."));
            emit executionFinished();
            return;
        }       

        if(!m_deviceModel || !m_deviceModel->receiverModel()) {
            setState(TState::TRuntimeError, tr("The target device is not initialized or was not found."));
            emit executionFinished();
            return;
        }

        log(QString(tr("[%1] Reading %2 bytes...")).arg(m_deviceModel->name()).arg(dataLen));

        connect(m_deviceModel->receiverModel(), &TReceiverModel::dataRead, this, &TScenarioIODeviceReadItem::dataRead);
        connect(m_deviceModel->receiverModel(), &TReceiverModel::readFailed, this, &TScenarioIODeviceReadItem::readFailed);
        connect(m_deviceModel->receiverModel(), &TReceiverModel::readBusy, this, &TScenarioIODeviceReadItem::readBusy);

        m_deviceModel->receiverModel()->readData(dataLen);
    }

    TConfigParam setParams(TConfigParam params) override {
        TConfigParam paramsToReturn = TScenarioIODeviceItem::setParams(params);

        m_title = "";
        m_subtitle = tr("no IO device selected");
        if(m_params.getState(true) != TConfigParam::TState::TError) {
            m_title = m_params.getSubParamByName("Component")->getValue() + ": read";
            m_subtitle = m_params.getSubParamByName("IO Device")->getValue();
        }

        emit appearanceChanged();
        return paramsToReturn;
    }
};

#endif // TSCENARIOIODEVICEREADITEM_H
