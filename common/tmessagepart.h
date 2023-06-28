#ifndef TMESSAGEPART_H
#define TMESSAGEPART_H
#include <QString>
#include <QList>
#include <QDataStream>
#include <QtGlobal>

#define TMESSAGEPARTVER "cz.cvut.fit.TraceXpert.TMessagePart/0.1"

/*!
 * \brief The TMessagePart class represents a single part of a TMessage. It is represented by its name, description, type, length, endianness and purpose (header/payload) in the message.
 *
 * The available types are defined by the TType enum: TString, TByteArray, TChar, TUChar (aka TByte), TInt, TUInt, TShort, TUShort, TLongLong, TULongLong, TReal, TBool.
 *
 * To retrieve the data of this message part for the final message buffer, the getData method which respects system and protocol endianness is to be used.
 *
 * If a variable length type (TString or TByteArray) is selected, a static or dynamic length can be specified using the length parameter.
 * In case of dynamic length, which can only be used for matching responses (not sending commands), the length parameter carries the index of another parameter in TMessage, which specifies the length.
 * The logic checks of these parameters are performed by TMessage.
 *
 * The value of the parameter is internally stored as a QByteArray and is obtained using the getValue method. The value of the parameter can be set using setValue methods.
 * The setValue methods perform checks on the set value with respect to the desired parameter type
 * (such as TInt or TShort; it checks whether the value can be converted to the desired type, then it stores it internally as a byte array).
 * Succesfully stored parameter value is signalled using the *ok.
 *
 * The whole clas can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 */
class TMessagePart {

    //Q_DECLARE_TR_FUNCTIONS(TMessagePart)

public:

    enum class TState {
        TOk,
        TInfo,
        TWarning,
        TError
    };

    enum class TType {
        TString,
        TByteArray,
        TChar,
        TUChar, /* aka */ TByte,
        TInt,
        TUInt,
        TShort,
        TUShort,
        TLongLong,
        TULongLong,
        TReal,
        TBool
    };

    TMessagePart() { }

    TMessagePart(const QString & name, const QString & description, TType type, bool isPayload = true, const QByteArray & value = {}, bool hasStaticLength = true, qsizetype length = 0, bool isLittleEndian = true) :
        m_name(name),
        m_description(description),
        m_type(type),
        m_isPayload(isPayload),
        m_value(value),
        m_hasStaticLength(hasStaticLength),
        m_length(length),
        m_isLittleEndian(isLittleEndian),
        m_state(TState::TOk),
        m_stateMessage()
    { }

    TMessagePart(const TMessagePart &x) :
        m_name(x.m_name),
        m_description(x.m_description),
        m_type(x.m_type),
        m_isPayload(x.m_isPayload),
        m_value(x.m_value),        
        m_hasStaticLength(x.m_hasStaticLength),
        m_length(x.m_length),
        m_isLittleEndian(x.m_isLittleEndian),
        m_state(x.m_state),
        m_stateMessage(x.m_stateMessage)
    {}

    TMessagePart & operator=(const TMessagePart &x){
        if(&x != this){
            m_name = x.m_name;
            m_description = x.m_description;
            m_type = x.m_type;
            m_isPayload = x.m_isPayload;
            m_value = x.m_value;
            m_hasStaticLength = x.m_hasStaticLength;
            m_length = x.m_length;
            m_isLittleEndian = x.m_isLittleEndian;
            m_state = x.m_state;
            m_stateMessage = x.m_stateMessage;
        }
        return *this;
    }

    bool operator==(const TMessagePart &x) const {
        return (m_name == x.m_name);
    }

    bool operator==(const QString &x) const {
        return (m_name == x);
    }

    friend QDataStream & operator<<(QDataStream &out, const TMessagePart &x) {
        out << QString(TMESSAGEPARTVER);
        out << x.m_name;
        out << x.m_description;
        out << x.m_type;
        out << x.m_isPayload;
        out << x.m_value;
        out << x.m_hasStaticLength;
        out << x.m_length;
        out << x.m_isLittleEndian;
        out << x.m_state;
        out << x.m_stateMessage;
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TMessagePart &x) {
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TMESSAGEPARTVER))){
            in >> x.m_name;
            in >> x.m_description;
            in >> x.m_type;
            in >> x.m_isPayload;
            in >> x.m_value;
            in >> x.m_hasStaticLength;
            in >> x.m_length;
            in >> x.m_isLittleEndian;
            in >> x.m_state;
            in >> x.m_stateMessage;
        } else {
            qCritical("Failed deserializing TMessagePart: Wrong version or wrong data.");
        }
        return in;
    }

    const QString & getName() const {
        return m_name;
    }
    const QString & getDescription() const {
        return m_description;
    }
    const enum TType getType() const {
        return m_type;
    }
    const bool hasStaticLength() const {
        switch(m_type) {
            case TType::TString:
            case TType::TByteArray:
                return m_hasStaticLength;
            default:
                return true;
        }
    }
    const qsizetype getLength() const {
        switch(m_type) {
            case TType::TString:
            case TType::TByteArray:
                return m_length;
            case TType::TChar:
            case TType::TUChar:
            case TType::TByte:
            case TType::TBool:
                return sizeof(char);
            case TType::TInt:
            case TType::TUInt:
                return sizeof(int);
            case TType::TShort:
            case TType::TUShort:
                return sizeof(short);
            case TType::TLongLong:
            case TType::TULongLong:
                return sizeof(long long);
            case TType::TReal:
                return sizeof(double);
            default:
                qWarning("TMessagePart has unrecognized type.");
                return -1;
        }
    }

    const bool isPayload() const {
        return m_isPayload;
    }
    const bool isLittleEndian() const {
        return m_isLittleEndian;
    }
    const QByteArray & getValue() const {
        return m_value;
    }

    const qsizetype hasLengthType() const {
        switch(m_type) {
            case TType::TChar:
            case TType::TUChar:
            case TType::TByte:
            case TType::TInt:
            case TType::TUInt:
            case TType::TShort:
            case TType::TUShort:
            case TType::TLongLong:
            case TType::TULongLong:
                return true;
            default:
                return false;
        }
    }

    const qsizetype getValueAsLength(bool *ok = nullptr) const {

        if(!this->hasLengthType()) {
            qWarning("TMessagePart does not have a suitable type to convert to qsizetype.");
            if(ok != nullptr) *ok = false;
        }

        if(this->hasStaticLength() && m_value.length() != this->getLength()) {
            qWarning("Value of TMessagePart is not the same as the length parameter; did you set the value?");
            if(ok != nullptr) *ok = false;
        }

        qsizetype length = m_value.toHex().toLongLong(ok, 16);

        return length;
    }

    const qsizetype getData(uint8_t * buffer, qsizetype maxLength) const {

        if(m_value.isEmpty()) {
            qWarning("TMessagePart value is empty; did you set the value?");
            return 0;
        }

        if(this->hasStaticLength() && m_value.length() != this->getLength()) {
            qWarning("Actual length of TMessagePart value is not the same as the length parameter; did you set the value?");
            return 0;
        }

        if(m_value.length() > maxLength) {
            qWarning("TMessagePart length is greater than supplied maxLength parameter.");
            return 0;
        }

        if(m_isLittleEndian) {
            std::copy(m_value.constBegin(), m_value.constEnd(), buffer);
        }
        else {
            std::reverse_copy(m_value.constBegin(), m_value.constEnd(), buffer);
        }

        return m_value.length();
    }

    const QByteArray & setValue(const QByteArray &value, bool *ok = nullptr) {
        // length of stored value is not checked id the value is dynamic, as we do not know the correct length
        bool iok = !this->hasStaticLength() || value.length() == getLength();

        if(iok == true){
            m_value = value;
        }

        if(ok != nullptr) *ok = iok;

        return m_value;
    }

    void setValue(qint8 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint8 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setByteValue(quint8 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(qint16 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint16 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(qint32 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint32 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(qint64 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint64 value, bool *ok = nullptr){
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setBool(bool value, bool *ok = nullptr){
        this->setValue(value ? QByteArrayLiteral("\x01") : QByteArrayLiteral("\x00"), ok);
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

    TMessagePart::TState getState() const{
        return m_state;
    }

    const QString & getStateMessage() const {
        return m_stateMessage;
    }

protected:
    QString m_name;
    QString m_description;
    TType m_type;
    bool m_isPayload;
    QByteArray m_value;
    bool m_hasStaticLength;
    qsizetype m_length;
    bool m_isLittleEndian;

    TState m_state;
    QString m_stateMessage;
};


#endif // TMESSAGEPART_H
