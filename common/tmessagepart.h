#ifndef TMESSAGEPART_H
#define TMESSAGEPART_H
#include <algorithm>
#include <QRegularExpression>
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
        TError,
        TUnevaluated
    };

    enum class TType {
        TString,
        TByteArray,
        TBool,
        TChar,
        TUChar, /* aka */ TByte,
        TShort,
        TUShort,
        TInt,
        TUInt,
        TLongLong,
        TULongLong,
        TReal
    };

    TMessagePart() : m_type(TType::TString), m_length(0) { }

    TMessagePart(const QString & name, const QString & description, TType type, bool isPayload = true, const QByteArray & value = {}, bool hasStaticLength = true, qsizetype length = 0, bool isLittleEndian = true) :
        m_name(name),
        m_description(description),
        m_type(type),
        m_isPayload(isPayload),
        m_value(value),
        m_hasStaticLength(hasStaticLength),
        m_length(length),
        m_isLittleEndian(isLittleEndian),
        m_state(TState::TUnevaluated),
        m_stateMessage("The message is yet to be evaluated.")
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

    enum TType getType() const {
        return m_type;
    }

    bool hasStaticLength() const {
        switch(m_type) {
            case TType::TString:
            case TType::TByteArray:
                return m_hasStaticLength;
            default:
                return true;
        }
    }

    qsizetype getLength() const {
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
                qWarning("TMessagePart \"%s\" has unrecognized type.", qPrintable(m_name));
                return -1;
        }
    }

    qsizetype getDataLength() const {
        return m_value.length();
    }

    bool isPayload() const {
        return m_isPayload;
    }

    bool isLittleEndian() const {
        return m_isLittleEndian;
    }

    bool hasLengthType() const {
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

    bool isHexOrAsciiSensibleType() const {
        switch(m_type) {
            case TType::TString:
            case TType::TByteArray:
            case TType::TChar:
            case TType::TUChar:
            case TType::TByte:
                return true;
            default:
                return false;
        }
    }

    const QByteArray getValue() const {
        if(m_isLittleEndian) {
            return m_value;
        }
        else {
            QByteArray reverse;
            reverse.reserve(m_value.size());
            for(int i = m_value.size()-1; i >= 0; i--) {
                reverse.append(m_value[i]);
            }
            return reverse;
        }
    }

    QString getHumanReadableValue() const {
        bool isFormattedAsHex;
        return getHumanReadableValue(isFormattedAsHex);
    }

    QString getHumanReadableValue(bool & isFormattedAsHex, bool preferHex = false) const {
        isFormattedAsHex = isHexOrAsciiSensibleType() && (preferHex || ((QString)m_value).contains(QRegularExpression(QStringLiteral("[^A-Za-z0-9]"))));

        if(isFormattedAsHex) {
            if(m_isLittleEndian) {
                return (QString)m_value.toHex();
            }
            else {
                QByteArray reverse;
                reverse.reserve(m_value.size());
                for(int i = m_value.size()-1; i >= 0; i--) {
                    reverse.append(m_value[i]);
                }
                return (QString)reverse.toHex();
            }
        }
        else if(isHexOrAsciiSensibleType()) {
            return (QString)m_value;
        }

        switch(m_type) {
            case TType::TShort: {
                    auto dataPointer = reinterpret_cast<const qint16 *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            case TType::TUShort: {
                    auto dataPointer = reinterpret_cast<const quint16 *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            case TType::TInt: {
                    auto dataPointer = reinterpret_cast<const qint32 *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            case TType::TUInt: {
                    auto dataPointer = reinterpret_cast<const quint32 *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            case TType::TLongLong: {
                    auto dataPointer = reinterpret_cast<const qint64 *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            case TType::TULongLong: {
                    auto dataPointer = reinterpret_cast<const quint64 *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            case TType::TBool: {
                    auto dataPointer = reinterpret_cast<const bool *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            case TType::TReal: {
                    auto dataPointer = reinterpret_cast<const double *>(m_value.constData());
                    return QString::number(*dataPointer);
                }
            default:
                return QStringLiteral("Could not interpret value.");
        }
    }

    qsizetype getValueAsLength(bool *ok = nullptr) const {

        if(!this->hasLengthType()) {
            qWarning("TMessagePart \"%s\" does not have a suitable type to convert to qsizetype.", qPrintable(m_name));
            if(ok != nullptr) *ok = false;
        }

        if(this->hasStaticLength() && m_value.length() != this->getLength()) {
            qWarning("Value of TMessagePart \"%s\" is not the same as the length parameter; did you set the value?", qPrintable(m_name));
            if(ok != nullptr) *ok = false;
        }

        // this needs to be done in order to interpret quint64 length values correctly
        // however, we're assigning quint64 into a qint64 which could cause problems,
        // but we can assume length will never be negative
        // and hopefully also not over quint64 max value ( :) )
        if(ok != nullptr) *ok = true;

        if(m_value.size() == 0) {
            return 0;
        }

        switch(m_type) {
            case TType::TChar: {
                    auto dataPointer = reinterpret_cast<const qint8 *>(m_value.constData());
                    return *dataPointer;
                }
            case TType::TUChar:
            case TType::TByte: {
                    auto dataPointer = reinterpret_cast<const quint8 *>(m_value.constData());
                    return *dataPointer;
                }
            case TType::TShort: {
                    auto dataPointer = reinterpret_cast<const qint16 *>(m_value.constData());
                    return *dataPointer;
                }
            case TType::TUShort: {
                    auto dataPointer = reinterpret_cast<const quint16 *>(m_value.constData());
                    return *dataPointer;
                }
            case TType::TInt: {
                    auto dataPointer = reinterpret_cast<const qint32 *>(m_value.constData());
                    return *dataPointer;
                }
            case TType::TUInt: {
                    auto dataPointer = reinterpret_cast<const quint32 *>(m_value.constData());
                    return *dataPointer;
                }
            case TType::TLongLong: {
                    auto dataPointer = reinterpret_cast<const qint64 *>(m_value.constData());
                    return *dataPointer;
                }
            case TType::TULongLong: {
                    auto dataPointer = reinterpret_cast<const quint64 *>(m_value.constData());
                    return *dataPointer;
                }
            default:
                if(ok != nullptr) *ok = false;
                return -1;
        }
    }

    QByteArray getData() const {

        if(m_value.isEmpty()) {
            qWarning("TMessagePart \"%s\" value is empty; did you set the value?", qPrintable(m_name));
            return QByteArray();
        }

        if(this->hasStaticLength() && m_value.length() != this->getLength()) {
            qWarning("Actual length of TMessagePart \"%s\" value is not the same as the length parameter; did you set the value?", qPrintable(m_name));
            return QByteArray();
        }

        if(m_isLittleEndian) {
            return m_value;
        }
        else {
            QByteArray reverse;
            reverse.reserve(m_value.size());
            for(int i = m_value.size()-1; i >= 0; i--) {
                reverse.append(m_value[i]);
            }
            return reverse;
        }
    }

    TMessagePart::TState getState() const{
        return m_state;
    }

    const QString & getStateMessage() const {
        return m_stateMessage;
    }

    void setName(const QString & value) {
        m_name = value;
    }

    void setDescription(const QString &  value) {
        m_description = value;
    }

    void setStaticLength(bool value) {
        m_hasStaticLength = value;
    }

    void setLength(qsizetype value) {
        m_length = value;
    }

    void setValue(const QByteArray & value, bool *ok = nullptr) {
        // length of stored value is not checked if the value is dynamic, as we do not know the correct length
        bool iok = !this->hasStaticLength() || value.length() == getLength();

        if(iok == true){
            m_value = value;
        }

        if(ok != nullptr) *ok = iok;
    }

    void setValue(const QString & value, bool *ok = nullptr, bool asHex = false, bool asAscii = false){
        static QRegularExpression asciiRegExp("^([\\x00-\\x7F])+$");
        static QRegularExpression hexRegExp("^([A-Fa-f0-9]|([A-Fa-f0-9]{2})+)$");

        if(!isHexOrAsciiSensibleType() && (asHex || asAscii)) {
            if(ok != nullptr) *ok = false;
            return;
        }

        bool iok = false;

        switch(m_type) {
            case TType::TByteArray:
            case TType::TByte: {
                QByteArray valueAsByteArray = value.toUtf8();
                if(asAscii) {
                    if(asciiRegExp.match(valueAsByteArray).hasMatch()) {
                        this->setValue(valueAsByteArray, &iok);
                    }
                }
                else {
                    if(hexRegExp.match(valueAsByteArray).hasMatch()) {
                        this->setValue(QByteArray::fromHex(valueAsByteArray), &iok);
                    }
                }
                break;
            }
            case TType::TString:
            case TType::TChar:
            case TType::TUChar: {
                QByteArray valueAsByteArray = value.toUtf8();
                if(asHex) {
                    if(hexRegExp.match(valueAsByteArray).hasMatch()) {
                        this->setValue(QByteArray::fromHex(valueAsByteArray), &iok);
                    }
                }
                else {
                    if(asciiRegExp.match(valueAsByteArray).hasMatch()) {
                        this->setValue(valueAsByteArray, &iok);
                    }
                }
                break;
            }
            case TType::TShort: {
                short convertedValue = value.toShort(&iok);
                if(iok) {
                    this->setValue(convertedValue, &iok);
                }
                break;
            }
            case TType::TUShort: {
                ushort convertedValue = value.toUShort(&iok);
                if(iok) {
                    this->setValue(convertedValue, &iok);
                }
                break;
            }
            case TType::TInt: {
                int convertedValue = value.toInt(&iok);
                if(iok) {
                    this->setValue(convertedValue, &iok);
                }
                break;
            }
            case TType::TUInt: {
                uint convertedValue = value.toUInt(&iok);
                if(iok) {
                    this->setValue(convertedValue, &iok);
                }
                break;
            }
            case TType::TLongLong: {
                qint64 convertedValue = value.toLongLong(&iok);
                if(iok) {
                    this->setValue(convertedValue, &iok);
                }
                break;
            }
            case TType::TULongLong: {
                quint64 convertedValue = value.toULongLong(&iok);
                if(iok) {
                    this->setValue(convertedValue, &iok);
                }
                break;
            }
            case TType::TReal: {
                double convertedValue = value.toDouble(&iok);
                if(iok) {
                    this->setValue(convertedValue, &iok);
                }
                break;
            }
            case TType::TBool: {
                if(value.compare("true", Qt::CaseInsensitive) == 0 || value == "1") {
                    this->setBool(true, &iok);
                }
                else if (value.compare("false", Qt::CaseInsensitive) == 0 || value == "0"){
                    this->setBool(false, &iok);
                }
                else {
                    iok = false;
                }
                break;
            }
            default:
                iok = false;
                break;
        }

        if(ok != nullptr) *ok = iok;
    }

    void setValue(qint8 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint8 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setByteValue(quint8 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(qint16 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint16 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(qint32 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint32 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(qint64 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(quint64 value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setValue(qreal value, bool *ok = nullptr) {
        this->setValue(QByteArray(reinterpret_cast<const char *>(&value), sizeof(value)), ok);
    }

    void setBool(bool value, bool *ok = nullptr) {
        this->setValue(value ? QByteArray::fromHex("01") : QByteArray::fromHex("00"), ok);
    }

    void setState(TState state) {
        m_state = state;
    }

    void setState(TState state, const QString &message) {
        m_state = state;
        m_stateMessage = message;
    }

    void resetState() {
        m_state = TState::TOk;
        m_stateMessage = "";
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
