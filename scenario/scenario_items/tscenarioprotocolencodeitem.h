#ifndef TSCENARIOPROTOCOLENCODEITEM_H
#define TSCENARIOPROTOCOLENCODEITEM_H

#include "../tscenarioitem.h"
#include "../../protocol/tprotocolmodel.h"
#include "../../protocol/tprotocol.h"

class TScenarioProtocolEncodeItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioProtocolEncodeItem;
    }

    TScenarioProtocolEncodeItem() : TScenarioItem(tr("Protocol: format message"), tr("This block formats a message from selected Protocol.")) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOut", "done", tr("Flow continues through this port on success."));
        addFlowOutputPort("flowOutError", "error", tr("Flow continues through this port on error."));
        addDataOutputPort("dataOut", "message", tr("Byte array containing the formatted message."), "[byte array]");

        m_params = TConfigParam("Block configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Protocol", "", TConfigParam::TType::TEnum, tr("Send message defined in this protocol."), false));
        m_params.addSubParam(TConfigParam("Message", "", TConfigParam::TType::TEnum, tr("Message to format using this block."), false));

        // item has to be initialized, otherwise it cannot be used
        m_subtitle = "no Protocol/Message selected";
        setState(TState::TError, tr("Block configuration contains errors!"));
    }

    TScenarioItem * copy() const override {
        return new TScenarioProtocolEncodeItem(*this);
    }

    const QString getIconResourcePath() const override {
        return ":/icons/message.png";
    }

    bool shouldUpdateParams(TConfigParam newParams) override {
        return isParamValueDifferent(newParams, m_params, "Protocol") ||
               isParamValueDifferent(newParams, m_params, "Message");
    }

    void updateParams(bool paramValuesChanged) override {
        TConfigParam * protocolParam = m_params.getSubParamByName("Protocol");
        protocolParam->clearEnumValues();
        protocolParam->resetState();

        int protocolCount = m_projectModel->protocolContainer()->count();
        int selectedProtocolIndex = protocolCount > 0 ? 0 : -1;
        for(int i = 0; i < protocolCount; i++) {
            QString protocolName = m_projectModel->protocolContainer()->at(i)->name();
            protocolParam->addEnumValue(protocolName);

            if(protocolName == protocolParam->getValue()) {
                selectedProtocolIndex = i;
            }
        }

        if(selectedProtocolIndex == -1) {
            protocolParam->setValue("");
            protocolParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }

        TConfigParam * messageParam = m_params.getSubParamByName("Message");
        messageParam->clearEnumValues();

        int selectedMessageIndex = -1;
        if(selectedProtocolIndex > -1) {
            messageParam->resetState();
            TProtocol * protocol = (TProtocol *)m_projectModel->protocolContainer()->at(selectedProtocolIndex)->unit();

            int messageCount = protocol->getMessages().count();
            selectedMessageIndex = messageCount > 0 ? 0 : -1;
            for(int i = 0; i < messageCount; i++) {
                const TMessage message = protocol->getMessages().at(i);

                if(message.isResponse())
                    continue;

                QString messageName = message.getName();
                messageParam->addEnumValue(messageName);

                if(messageName == messageParam->getValue()) {
                    selectedMessageIndex = i;
                }
            }

            if(selectedMessageIndex > -1) {
                messageParam->setValue(protocol->getMessages().at(selectedMessageIndex).getName());
            }
        }

        if(selectedMessageIndex == -1) {
            messageParam->setValue("");
            messageParam->setState(TConfigParam::TState::TError, tr("Selected value is invalid!"));
        }
    }

    bool validateParamsStructure(TConfigParam params) override {
        bool iok = false;

        params.getSubParamByName("Protocol", &iok);
        if(!iok) return false;

        params.getSubParamByName("Message", &iok);
        if(!iok) return false;

        return true;
    }

    TConfigParam setParams(TConfigParam params) override {
        if(!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the pre-init params."));
            return params;
        }

        bool shouldUpdate = shouldUpdateParams(params);
        m_params = params;
        updateParams(shouldUpdate);

        if(m_params.getState(true) == TConfigParam::TState::TError) {
            setState(TState::TError, tr("Block configuration contains errors!"));
        }
        else {
            resetState();
        }

        for(QString & portName : m_generatedPortNames) {
            removePort(portName);
        }
        m_generatedPortNames.clear();

        bool iok;
        TMessage message = getMessage(&iok);

        if(iok) {
            m_subtitle = message.getName();
            for(TMessagePart & messagePart : message.getMessageParts()) {
                if(messagePart.isPayload()) {
                    addDataInputPort(messagePart.getName(), messagePart.getName(), messagePart.getDescription());
                    m_generatedPortNames.append(messagePart.getName());
                }
            }
            emit appearanceChanged();
        }
        else {
            setState(TState::TError, tr("Failed to obtain selected protocol message, is it available?"));
        }

        return m_params;
    }

    bool prepare() override {
        bool iok;
        getMessage(&iok);

        if(!iok) {
            setState(TState::TError, tr("Failed to obtain selected protocol message, is it available?"));
            return false;
        }

        return true;
    }

    QHash<TScenarioItemPort *, QByteArray> executeDirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {

        bool iok;
        TMessage message = getMessage(&iok);

        if(!iok) {
            setState(TState::TRuntimeError, tr("Failed to obtain selected protocol message, is it available?"));
            return QHash<TScenarioItemPort *, QByteArray>();
        }

        for(TMessagePart & messagePart : message.getMessageParts()) {
            if(!messagePart.isPayload())
                continue;

            messagePart.setValue(inputData.value(getItemPortByName(messagePart.getName())), &iok);

            if(!iok) {
                log(QString("Failed to set value of \"%1\" message part.").arg(messagePart.getName()), TLogLevel::TWarning);
                setState(TState::TRuntimeWarning, tr("Failed to set one or more message part values!"));
            }
        }

        if(message.getState() != TMessage::TState::TOk) {
            setState(TState::TRuntimeWarning, message.getStateMessage());
        }

        QHash<TScenarioItemPort *, QByteArray> outputData;
        outputData.insert(getItemPortByName("dataOut"), message.getData());
        return outputData;
    }

    TMessage getMessage(bool * ok) {
        bool iok;

        TConfigParam * protocolParam = m_params.getSubParamByName("Protocol");
        TProtocolModel * protocolModel = (TProtocolModel *)m_projectModel->protocolContainer()->getByName(protocolParam->getValue());

        if(!protocolModel) {
            if(ok != nullptr) *ok = false;
            return TMessage();
        }

        TConfigParam * messageParam = m_params.getSubParamByName("Message");
        TMessage message = protocolModel->protocol()->getMessageByName(messageParam->getValue(), &iok);

        if(!iok) {
            if(ok != nullptr) *ok = false;
            return TMessage();
        }

        if(ok != nullptr) *ok = true;
        return message;
    }

    TScenarioItemPort * getPreferredOutputFlowPort() override {
        return m_state != TState::TOk ? this->getItemPortByName("flowOutError") : this->getItemPortByName("flowOut");
    }

protected:
    QList<QString> m_generatedPortNames;
};

#endif // TSCENARIOPROTOCOLENCODEITEM_H
