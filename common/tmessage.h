#ifndef TMESSAGE_H
#define TMESSAGE_H
#include <QString>
#include <QList>
#include <QDataStream>
#include <QtGlobal>
#include "tmessagepart.h"

#define TMESSAGEVER "cz.cvut.fit.TraceXpert.TMessage/0.1"

/*!
 * \brief The TMessage class represents a protocol message with its name, description, purpose (command or response) and state.
 *
 * A protocol message consists of message parts, which are kept in order in a protected list. User can add, remove, insert and get message parts.
 *
 * If the message is supposed to act as a command, after all message parts are in place, the user can supply values to them and use the getData method to fill a buffer with the raw command byte data.
 * It is however important to check the validity of the message (call checkValidity and then check the state), especially when data is modified through the getMessagePartByName method.
 *
 * If on the other hand the message is supposed to be a response, the user can use TProtocol, match the message and then extract its important parts.
 *
 * The whole class can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 */
class TMessage {

    //Q_DECLARE_TR_FUNCTIONS(TMessage)

public:

    enum class TState {
        TOk,
        TInfo,
        TWarning,
        TError
    };

    TMessage() { }

    TMessage(const QString & name, const QString & description, bool isResponse = false):
        m_description(description),
        m_isResponse(isResponse),
        m_messageParts(),
        m_state(TState::TOk),
        m_stateMessage()
    { }


    TMessage(const TMessage &x):
        m_description(x.m_description),
        m_isResponse(x.m_isResponse),
        m_messageParts(x.m_messageParts),
        m_state(x.m_state),
        m_stateMessage(x.m_stateMessage)
    { }

    TMessage & operator=(const TMessage &x){
        if(&x != this){
            m_description = x.m_description;
            m_isResponse = x.m_isResponse;
            m_messageParts = x.m_messageParts;
            m_state = x.m_state;
            m_stateMessage = x.m_stateMessage;
        }
        return *this;
    }

    bool operator==(const TMessage &x) const {
        return (m_name == x.m_name);
    }
    bool operator==(const QString &x) const {
        return (m_name == x);
    }

    friend QDataStream & operator<<(QDataStream &out, const TMessage &x){
        out << QString(TMESSAGEVER);
        out << x.m_description;
        out << x.m_isResponse;
        out << x.m_messageParts;
        out << x.m_state;
        out << x.m_stateMessage;
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TMessage &x){
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TMESSAGEVER))){
            in >> x.m_description;
            in >> x.m_isResponse;
            in >> x.m_messageParts;
            in >> x.m_state;
            in >> x.m_stateMessage;
        } else {
            qCritical("Failed deserializing TMessage: Wrong version or wrong data.");
        }
        return in;
    }

    const QString & getName() const {
        return m_name;
    }
    const QString & getDescription() const {
        return m_description;
    }
    const bool isResponse() const {
        return m_isResponse;
    }

    void setState(TState state){
        m_state = state;
    }

    void setState(TState state, const QString &message){
        m_state = state;
        m_stateMessage = message;
    }

    void resetState(){
        m_state = TState::TOk;
        m_stateMessage = "";
    }

    TMessage::TState getState() const{
        return m_state;
    }

    const QString & getStateMessage() const {
        return m_stateMessage;
    }

    void validateMessage() {
        resetState();

        bool hasErrors = false;
        for(int i = 0; i < m_messageParts.length(); i++) {
            TMessagePart & messagePart = m_messageParts[i];
            messagePart.resetState();

            if(!m_isResponse && messagePart.hasStaticLength() && messagePart.getLength() != messagePart.getValue().length()) {
                messagePart.setState(TMessagePart::TState::TWarning, "Actual length of TMessagePart value is not the same as the length parameter; did you set the value?");
                hasErrors = true;
            }

            if(!m_isResponse && !messagePart.hasStaticLength()) {
                messagePart.setState(TMessagePart::TState::TError, "TMessage type is set as command, not response, it is not possible to have dynamic length message parts!");
                hasErrors = true;
            }

            if(m_isResponse && !messagePart.hasStaticLength()) {
                if(messagePart.getLength() >= i) {
                    messagePart.setState(TMessagePart::TState::TError, "Dynamic length message parts must have their lengths specified by preceding parameters!");
                    hasErrors = true;
                }

                if(!m_messageParts.at(messagePart.getLength()).hasStaticLength()) {
                    messagePart.setState(TMessagePart::TState::TError, "Dynamic length message parts must have their lengths specified by parameters with static length!");
                    hasErrors = true;
                }

                if(!m_messageParts.at(messagePart.getLength()).hasLengthType()) {
                    messagePart.setState(TMessagePart::TState::TError, "Dynamic length message parts must have their lengths specified by numeric parameters!");
                    hasErrors = true;
                }
            }
        }

        if(hasErrors) {
            m_state = TState::TError;
            m_stateMessage = "One ore more message parts have errors that need to be fixed.";
        }
    }

    const qsizetype getLength() const {

        if(m_state == TState::TError) {
            qWarning("Cannot find length of invalid message, fix errors first.");
            return -1;
        }

        qsizetype length = 0;
        for(int i = 0; i < m_messageParts.length(); i++) {
            const TMessagePart & messagePart = m_messageParts.at(i);

            if(messagePart.hasStaticLength()) {
                qsizetype partLength = messagePart.getLength();

                if(partLength < 0) {
                    qWarning("Cannot find length of message; a message part has negative length.");
                    return -1;
                }

                length += partLength;
            }
            else {
                bool ok;
                qsizetype partLength = m_messageParts[messagePart.getLength()].getValueAsLength(&ok);

                if(!ok) {
                    qWarning("Cannot find length of message; a message part specifying length of another message part has an invalid value.");
                    return -1;
                }

                if(partLength < 0) {
                    qWarning("Cannot find length of message; a message part specifying length of another message part has a negative value.");
                    return -1;
                }

                length += partLength;
            }
        }

        return length;
    }

    const qsizetype getData(uint8_t * buffer, qsizetype maxLength) const {

        if(m_isResponse) {
            qWarning("Getting data of a message which is a response, not a command.");
        }

        if(m_state == TState::TError) {
            qWarning("Cannot get data of invalid message, fix errors first.");
            return -1;
        }

        qsizetype messageLength = 0;

        for(int i = 0; i < m_messageParts.length(); i++) {
            const TMessagePart & messagePart = m_messageParts.at(i);

            qsizetype appendedBytes = messagePart.getData(buffer + messageLength, maxLength - messageLength);

            if(appendedBytes == 0) {
                qWarning("Failed to form message as a result of an error; check previous messages.");
                return 0;
            }

            messageLength += appendedBytes;
        }

        if(messageLength != getLength()) {
            qWarning("Failed to form message of correct length; check previous messages.");
            return -1;
        }

        return messageLength;
    }

    QList<TMessagePart> & getMessageParts(){
        return m_messageParts;
    }

    void addMessagePart(const TMessagePart &param, bool *ok = nullptr){
        if(m_messageParts.contains(param)){
            if(ok != nullptr) *ok = false;
            return;
        }
        m_messageParts.append(param);
        if(ok != nullptr) *ok = true;

        validateMessage();
    }

    void insertMessagePart(const TMessagePart &param, int pos = -1, bool *ok = nullptr){
        if(m_messageParts.contains(param)){
            if(ok != nullptr) *ok = false;
            return;
        }
        if(pos < 0 || pos > m_messageParts.size()) {
            pos = m_messageParts.size();
        }
        m_messageParts.insert(pos, param);
        if(ok != nullptr) *ok = true;

        validateMessage();
    }

    void removeMessagePart(const QString &name, bool *ok = nullptr){
        qsizetype noOfRemoved = m_messageParts.removeAll(name);
        if(ok != nullptr && noOfRemoved > 0){
            *ok = true;
        } else if(ok != nullptr){
            *ok = false;
        }

        validateMessage();
    }

    TMessagePart * getMessagePartByName(const QString &name, bool *ok = nullptr) {
        int index = this->getMessageParts().indexOf(name);
        if(index < 0){
            if(ok != nullptr) *ok = false;
            return nullptr;
        } else {
            if(ok != nullptr) *ok = true;
            return &(this->getMessageParts()[index]);
        }
    }

    qsizetype getMessagePartLengthByName(const QString &name, bool *ok = nullptr) {
        if(m_state == TState::TError) {
            qWarning("Cannot get message part length of invalid message, fix errors first.");
            if(ok != nullptr) *ok = false;
            return -1;
        }

        int index = this->getMessageParts().indexOf(name);
        if(index < 0){
            if(ok != nullptr) *ok = false;
            return -1;
        } else {
            if(ok != nullptr) *ok = true;
            TMessagePart & messagePart = this->getMessageParts()[index];
            return messagePart.hasStaticLength() ? messagePart.getLength() : this->getMessageParts()[messagePart.getLength()].getValueAsLength(ok);
        }
    }

protected:
    QString m_name;
    QString m_description;
    bool m_isResponse;

    QList<TMessagePart> m_messageParts;

    TState m_state;
    QString m_stateMessage;
};


#endif // TMESSAGE_H
