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
        m_name(name),
        m_description(description),
        m_isResponse(isResponse),
        m_messageParts(),
        m_state(TState::TOk),
        m_stateMessage()
    { }


    TMessage(const TMessage &x):
        m_name(x.m_name),
        m_description(x.m_description),
        m_isResponse(x.m_isResponse),
        m_messageParts(x.m_messageParts),
        m_state(x.m_state),
        m_stateMessage(x.m_stateMessage)
    { }

    TMessage & operator=(const TMessage &x){
        if(&x != this){
            m_name = x.m_name;
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
        out << x.m_name;
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
        if(Q_LIKELY(verString == QString(TMESSAGEVER))) {
            in >> x.m_name;
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

    TMessage::TState getState() const{
        return m_state;
    }

    const QString & getStateMessage() const {
        return m_stateMessage;
    }

    void validateMessage() {
        resetState();

        bool hasWarnings = false;
        bool hasErrors = false;
        for(int i = 0; i < m_messageParts.length(); i++) {
            TMessagePart & messagePart = m_messageParts[i];
            messagePart.resetState();

            if(!m_isResponse && messagePart.hasStaticLength() && !messagePart.isPayload() && messagePart.getLength() != messagePart.getValue().length()) {
                messagePart.setState(TMessagePart::TState::TWarning, "Actual length of TMessagePart value is not the same as the length parameter; did you set the value?");
                hasWarnings = true;
            }

            if(!messagePart.hasStaticLength()) {
                if(messagePart.getLength() >= i || messagePart.getLength() < 0) {
                    messagePart.setState(TMessagePart::TState::TError, "Dynamic length message parts must have their lengths specified by preceding parameters!");
                    hasErrors = true;
                    continue;
                }

                if(!m_messageParts[messagePart.getLength()].hasStaticLength()) {
                    messagePart.setState(TMessagePart::TState::TError, "Dynamic length message parts must have their lengths specified by parameters with static length!");
                    hasErrors = true;
                }

                if(!m_messageParts[messagePart.getLength()].hasLengthType()) {
                    messagePart.setState(TMessagePart::TState::TError, "Dynamic length message parts must have their lengths specified by numeric parameters!");
                    hasErrors = true;
                }
            }
        }

        if(hasErrors) {
            m_state = TState::TError;
            m_stateMessage = "One ore more message parts have errors that need to be fixed.";
        }
        else if (hasWarnings) {
            m_state = TState::TWarning;
            m_stateMessage = "One ore more message parts have warnings that may need attention.";
        }
    }

    qsizetype getLength() const {

        if(m_state == TState::TError) {
            qWarning("Cannot find length of invalid message, fix errors first.");
            return -1;
        }

        qsizetype length = 0;
        for(int i = 0; i < m_messageParts.length(); i++) {
            const TMessagePart & messagePart = m_messageParts[i];

            if(messagePart.hasStaticLength()) {
                qsizetype partLength = messagePart.getLength();

                if(partLength < 0) {
                    qWarning("Cannot find length of message; message part %s has negative length.", qPrintable(messagePart.getName()));
                    return -1;
                }

                length += partLength;
            }
            else {
                qsizetype referencedIndex = messagePart.getLength();
                if(referencedIndex >= i || referencedIndex < 0) {
                    qWarning("Could not get dynamic length of message part %s; referenced message part does not exist.", qPrintable(messagePart.getName()));
                    return -1;
                }

                bool ok;
                qsizetype partLength = m_messageParts[referencedIndex].getValueAsLength(&ok);

                if(!ok) {
                    qWarning("Cannot find length of message; message part %s specifying length of message part %s has an invalid value.", qPrintable(m_messageParts[referencedIndex].getName()), qPrintable(messagePart.getName()));
                    return -1;
                }

                if(partLength < 0) {
                    qWarning("Cannot find length of message; message part %s specifying length of message part %s has a negative value.", qPrintable(m_messageParts[referencedIndex].getName()), qPrintable(messagePart.getName()));
                    return -1;
                }

                length += partLength;
            }
        }

        return length;
    }

    QByteArray getData() const {

        if(m_isResponse) {
            qWarning("Getting data of a message \"%s\" which is a response, not a command.", qPrintable(m_name));
        }

        if(m_state == TState::TError) {
            qWarning("Cannot get data of invalid message \"%s\", fix errors first.", qPrintable(m_name));
            return QByteArray();
        }

        QByteArray messageData;

        for(int i = 0; i < m_messageParts.length(); i++) {
            const TMessagePart & messagePart = m_messageParts[i];
            const QByteArray & messagePartData = messagePart.getData();

            if(messagePartData.length() == 0)
                break;

            messageData.append(messagePartData);
        }

        if(messageData.length() != getLength()) {
            qWarning("Failed to form message \"%s\" of correct length; check previous messages.", qPrintable(m_name));
            return QByteArray();
        }

        return messageData;
    }

    void addMessagePart(const TMessagePart &param, bool *ok = nullptr) {
        if(m_messageParts.contains(param)){
            if(ok != nullptr) *ok = false;
            return;
        }
        m_messageParts.append(param);
        if(ok != nullptr) *ok = true;

        validateMessage();
    }

    void insertMessagePart(const TMessagePart &param, int pos = -1, bool *ok = nullptr) {
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

    void removeMessagePart(const QString &name, bool *ok = nullptr) {
        qsizetype noOfRemoved = m_messageParts.removeAll(name);
        if(ok != nullptr && noOfRemoved > 0){
            *ok = true;
        } else if(ok != nullptr){
            *ok = false;
        }

        validateMessage();
    }

    TMessagePart getMessagePartByName(const QString &name, bool *ok = nullptr) const {
        int index = m_messageParts.indexOf(name);
        if(index < 0){
            if(ok != nullptr) *ok = false;
            return TMessagePart();
        } else {
            if(ok != nullptr) *ok = true;
            return m_messageParts[index];
        }
    }

    qsizetype getMessagePartLengthByName(const QString &name, bool *ok = nullptr) const {
        if(m_state == TState::TError) {
            qWarning("Cannot get message part length of invalid message \"%s\", fix errors first.", qPrintable(m_name));
            if(ok != nullptr) *ok = false;
            return -1;
        }

        int index = m_messageParts.indexOf(name);
        if(index < 0){
            if(ok != nullptr) *ok = false;
            return -1;
        }

        const TMessagePart & messagePart = m_messageParts[index];

        if(messagePart.hasStaticLength()) {
            if(ok != nullptr) *ok = true;
            return messagePart.getLength();
        }

        qsizetype referencedIndex = messagePart.getLength();
        if(referencedIndex >= index || referencedIndex < 0) {
            qWarning("Could not get dynamic length of message part %s; referenced message part does not exist.", qPrintable(messagePart.getName()));
            if(ok != nullptr) *ok = false;
            return -1;
        }

        if(ok != nullptr) *ok = true;
        return m_messageParts[messagePart.getLength()].getValueAsLength(ok);
    }

    QString getPayloadSummary() const {

        if(m_messageParts.isEmpty()) {
            return m_name;
        }

        QList<QString> messagePartSummaries;
        for(const TMessagePart & messagePart : m_messageParts) {
            if(!messagePart.isPayload())
                continue;

            bool isFormattedAsHex;
            QString value = messagePart.getHumanReadableValue(isFormattedAsHex);
            messagePartSummaries.append(QString(isFormattedAsHex ? "%1: 0x%2" : "%1: %2").arg(messagePart.getName(), value));
        }

        if(messagePartSummaries.isEmpty()) {
            return m_name;
        }

        return QString("%1: (%2)").arg(m_name, messagePartSummaries.join(", "));
    }

    QList<TMessagePart> & getMessageParts() {
        return m_messageParts;
    }

    const QList<TMessagePart> & getMessageParts() const {
        return m_messageParts;
    }

    void setName(const QString &  value) {
        m_name = value;
    }

    void setDescription(const QString &  value) {
        m_description = value;
    }

    void setResponse(bool value) {
        m_isResponse = value;
    }

    void setState(TState state){
        m_state = state;
    }

    void setState(TState state, const QString & message){
        m_state = state;
        m_stateMessage = message;
    }

    void resetState(){
        m_state = TState::TOk;
        m_stateMessage = "";
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
