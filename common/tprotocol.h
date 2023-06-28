#ifndef TPROTOCOL_H
#define TPROTOCOL_H
#include <QString>
#include <QList>
#include <QDataStream>
#include <QtGlobal>
#include "tmessage.h"

#define TPROTOCOLVER "cz.cvut.fit.TraceXpert.TProtocol/0.1"

/*!
 * \brief The TProtocol class represents a protocol with its name, description and a set of its messages.
 *
 * The tryMatchResponse method is used to match incoming protocol data to predefined "response" type messages.
 *
 * The whole class, including subparameters etc., can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 */
class TProtocol {

    //Q_DECLARE_TR_FUNCTIONS(TProtocol)

public:

    TProtocol() { }

    TProtocol(const QString &name, const QString &description, QList<TMessage> messages = {}):
        m_name(name),
        m_description(description),
        m_messages(messages)
    { }

    TProtocol(const TProtocol &x):
        m_name(x.m_name),
        m_description(x.m_description),
        m_messages(x.m_messages)
    { }

    TProtocol & operator=(const TProtocol &x){
        if(&x != this){
            m_name = x.m_name;
            m_description = x.m_description;
            m_messages = x.m_messages;
        }
        return *this;
    }

    bool operator==(const TProtocol &x) const {
        return (m_name == x.m_name);
    }
    bool operator==(const QString &x) const {
        return (m_name == x);
    }

    friend QDataStream & operator<<(QDataStream &out, const TProtocol &x){
        out << QString(TPROTOCOLVER);
        out << x.m_name;
        out << x.m_description;
        out << x.m_messages;
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TProtocol &x){
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TPROTOCOLVER))){
            in >> x.m_name;
            in >> x.m_description;
            in >> x.m_messages;
        } else {
            qCritical("Failed deserializing TProtocol: Wrong version or wrong data.");
        }
        return in;
    }

    const QString & getName() const {
        return m_name;
    }

    const QString & getDescription() const {
        return m_description;
    }

    const TMessage * tryMatchResponse(uint8_t * buffer, qsizetype length) {
        QByteArray receivedData = QByteArray(reinterpret_cast<const char *>(buffer), length);

        TMessage * matchedMessage = nullptr;

        for(TMessage message : m_messages) {

            if(!message.isResponse()) {
                continue;
            }

            if(message.getState() == TMessage::TState::TError) {
                qWarning("Skipping matching of message with Error state!");
                continue;
            }

            bool iok, isMatch = true;
            qsizetype len, pos = 0;

            for(TMessagePart & messagePart : message.getMessageParts()) {
                len = message.getMessagePartLengthByName(messagePart.getName(), &iok);

                if(!iok) {
                    // qWarning("Skipping matching of message, could not get length of message part!");
                    isMatch = false;
                    break;
                }

                if(pos + len > length) {
                    // qInfo("Candidate message longer then received message...");
                    isMatch = false;
                    break;
                }

                if(messagePart.isPayload()) {
                    if(messagePart.isLittleEndian()) {
                        messagePart.setValue(receivedData.sliced(pos, len), &iok);
                    }
                    else {
                        QByteArray tmp = receivedData.sliced(pos, len);
                        std::reverse(tmp.begin(), tmp.end());
                        messagePart.setValue(tmp, &iok);
                    }

                    if(!iok) {
                        qWarning("Failed to fill in payload while matching message!");
                        isMatch = false;
                        break;
                    }
                    else {
                        // qInfo("Payload filled in ok...");
                    }
                }
                else {
                    if(messagePart.getValue() != receivedData.sliced(pos, len)) {
                        // qInfo("Header message part does not match...");
                        isMatch = false;
                        break;
                    }
                    else {
                        // qInfo("Header message part match!");
                    }
                }

                pos += len;
            }

            if(isMatch && pos != length) {
                // qInfo("Could be a match, but received message has yet more data...");
                continue;
            }

            if(isMatch) {
                matchedMessage = new TMessage(message);
            }
        }

        return matchedMessage;
    }

    QList<TMessage> & getMessages(){
        return m_messages;
    }

    void addMessage(const TMessage &param, bool *ok = nullptr){
        if(m_messages.contains(param)){
            if(ok != nullptr) *ok = false;
            return;
        }
        m_messages.append(param);
        if(ok != nullptr) *ok = true;
    }

    void removeMessage(const QString &name, bool *ok = nullptr){
        qsizetype noOfRemoved = m_messages.removeAll(name);
        if(ok != nullptr && noOfRemoved > 0){
            *ok = true;
        } else if(ok != nullptr){
            *ok = false;
        }
    }

    TMessage * getMessageByName(const QString &name, bool *ok = nullptr){
        int index = this->getMessages().indexOf(name);
        if(index < 0){
            if(ok != nullptr) *ok = false;
            return nullptr;
        } else {
            if(ok != nullptr) *ok = true;
            return &(this->getMessages()[index]);
        }
    }

protected:
    QString m_name;
    QString m_description;
    QList<TMessage> m_messages;
};


#endif // TPROTOCOL_H
