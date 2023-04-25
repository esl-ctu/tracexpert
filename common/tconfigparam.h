#ifndef TCONFIGPARAM_H
#define TCONFIGPARAM_H
#include <QString>
#include <QList>
#include <QDataStream>
#include <QtGlobal>

#define TCONFIGPARAMVER "cz.cvut.fit.TraceXpert.TConfigParam/0.1"

/*!
 * \brief The TConfigParam class represents a parameter with its name, default value, type and a hint for the user.
 *
 * The available types are defined by the TType enum: TString, TInt, TUInt, TShort, TUShort, TLongLong, TULongLong, TDouble, TBool, TDummy, TEnum.
 *
 * Parameter with the TDummy type does not allow any value to be set.
 *
 * Parameter with the TEnum type allows only predefined values. These are predefined using the addEnumValue and removeEnumValue methods, and obtained using the getEnumValues method.
 *
 * The value of the parameter is internally stored as a QString and is obtained using the getValue method. The value of the parameter can be set using setValue methods. The setValue methods perform checks on the set value with respect to the desired parameter type (such as TInt or TEnum; it checks whether the value can be converted to the desired type, then it stores it internally as a string). Succesfully stored parameter value is signalled using the *ok.
 *
 * The parameter may have a warning raised (methods setWarning, resetWarning, isWarning, getWarning). This is for convenience of the end-user.
 *
 * The parameter may contain arbitrary number of subparameters (addSubParam, removeSubParam, getSubParams, getSubParamByName). This allows for a hierarchy of parameters to be stored within a single instance.
 *
 * The whole class, including subparameters etc., can be serialized and deserialized through QDataStream by using \<\< and \>\> operators.
 */
class TConfigParam {

    //Q_DECLARE_TR_FUNCTIONS(TConfigParam)

public:

    enum class TType {
        TString,
        TInt,
        TUInt,
        TShort,
        TUShort,
        TLongLong,
        TULongLong,
        TReal,
        TBool,
        TDummy,
        TEnum,
        TFileName,
        TTime
    };

    enum class TState {
        TOk,
        TInfo,
        TWarning,
        TError
    };

    TConfigParam(){

    }
    TConfigParam(const QString &name, const QString &defaultValue, enum TType type, const QString &hint, bool readonly = false): m_name(name), m_defaultValue(defaultValue), m_value(defaultValue), m_type(type), m_hint(hint), m_state(TState::TOk), m_stateMessage(), m_readonly(readonly), m_enums(), m_subParams() {

    }
    TConfigParam(const TConfigParam &x): m_name(x.m_name), m_defaultValue(x.m_defaultValue), m_value(x.m_value), m_type(x.m_type), m_hint(x.m_hint), m_state(x.m_state), m_stateMessage(x.m_stateMessage), m_readonly(x.m_readonly), m_enums(x.m_enums), m_subParams(x.m_subParams) {

    }

    TConfigParam & operator=(const TConfigParam &x){
        if(&x != this){
            m_name = x.m_name;
            m_defaultValue = x.m_defaultValue;
            m_value = x.m_value;
            m_type = x.m_type;
            m_hint = x.m_hint;
            m_state = x.m_state;
            m_stateMessage = x.m_stateMessage;
            m_readonly = x.m_readonly;
            m_enums = x.m_enums;
            m_subParams = x.m_subParams;
        }
        return *this;
    }

    bool operator==(const TConfigParam &x) const {
        return (m_name == x.m_name);
    }
    bool operator==(const QString &x) const {
        return (m_name == x);
    }

    friend QDataStream & operator<<(QDataStream &out, const TConfigParam &x){
        out << QString(TCONFIGPARAMVER);
        out << x.m_name;
        out << x.m_defaultValue;
        out << x.m_value;
        out << x.m_type;
        out << x.m_hint;
        out << x.m_state;
        out << x.m_stateMessage;
        out << x.m_readonly;
        out << x.m_enums;
        out << x.m_subParams;
        return out;
    }

    friend QDataStream & operator>>(QDataStream &in, TConfigParam &x){
        QString verString;
        in >> verString;
        if(Q_LIKELY(verString == QString(TCONFIGPARAMVER))){
            in >> x.m_name;
            in >> x.m_defaultValue;
            in >> x.m_value;
            in >> x.m_type;
            in >> x.m_hint;
            in >> x.m_state;
            in >> x.m_stateMessage;
            in >> x.m_readonly;
            in >> x.m_enums;
            in >> x.m_subParams;
        } else {
            qCritical("Failed deserializing TConfigParam: Wrong version or wrong data.");
        }
        return in;
    }

    const QString & getName() const {
        return m_name;
    }
    const QString & getValue() const {
        return m_value;
    }
    const QString & getDefaultValue() const {
        return m_defaultValue;
    }
    const enum TType getType() const {
        return m_type;
    }
    const QString & getHint() const {
        return m_hint;
    }

    const QString & setValue(const QString &value, bool *ok = nullptr){

        bool iok=true;

        switch(m_type) {
            case TType::TString:
            case TType::TFileName:
                iok = true;
                break;
            case TType::TInt:
                value.toInt(&iok);
                break;
            case TType::TUInt:
                value.toUInt(&iok);
                break;
            case TType::TShort:
                value.toShort(&iok);
                break;
            case TType::TUShort:
                value.toUShort(&iok);
                break;
            case TType::TLongLong:
                value.toLongLong(&iok);
                break;
            case TType::TULongLong:
                value.toULongLong(&iok);
                break;
            case TType::TReal:
            case TType::TTime:
                value.toDouble(&iok);
                break;
            case TType::TBool:
                if(value == "true" || value == "false"){
                    iok = true;
                } else {
                    iok = false;
                }
                break;
            case TType::TDummy:
                iok = false;
                break;
            case TType::TEnum:
                iok = m_enums.contains(value);
                break;
            default:
                iok = false;
                break;
        }

        if(m_readonly){
            iok = false;
        }

        if(iok == true){
            m_value = value;
        }

        if(ok != nullptr) *ok = iok;

        return m_value;
    }

    qint32 setValue(qint32 value, bool *ok = nullptr){
        bool iok;
        int ret = this->setValue(QString::number(value), ok).toInt(&iok);
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret;
    }

    quint32 setValue(quint32 value, bool *ok = nullptr){
        bool iok;
        uint ret = this->setValue(QString::number(value), ok).toUInt(&iok);
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret;
    }

    qint16 setValue(qint16 value, bool *ok = nullptr){
        bool iok;
        short ret = this->setValue(QString::number(value), ok).toShort(&iok);
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret;
    }

    quint16 setValue(quint16 value, bool *ok = nullptr){
        bool iok;
        ushort ret = this->setValue(QString::number(value), ok).toUShort(&iok);
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret;
    }

    qint64 setValue(qint64 value, bool *ok = nullptr){
        bool iok;
        qlonglong ret = this->setValue(QString::number(value), ok).toLongLong(&iok);
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret;
    }

    quint64 setValue(quint64 value, bool *ok = nullptr){
        bool iok;
        qulonglong ret = this->setValue(QString::number(value), ok).toULongLong(&iok);
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret;
    }

    qreal setValue(qreal value, bool *ok = nullptr){
        bool iok;
        qreal ret = this->setValue(QString::number(value), ok).toDouble(&iok);
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret;
    }

    bool setBool(bool value, bool *ok = nullptr){
        bool iok;
        QString ret = this->setValue(QString(value ? "true" : "false"), ok);
        if(ret == "true" || ret == "false"){
            iok = true;
        } else {
            iok = false;
        }
        if(ok != nullptr){
            *ok = *ok && iok;
        }
        return ret == "true" ? true : false;
    }

    void resetDefaultValue(){
        m_value = m_defaultValue;
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
    TConfigParam::TState getState() const{
        return m_state;
    }
    const QString & getStateMessage() const {
        return m_stateMessage;
    }


    bool isReadonly() const {
        return m_readonly;
    }

    const QList<QString> & getEnumValues() const{
        return m_enums;
    }

    void addEnumValue(const QString &value, bool *ok = nullptr){
        if(m_type != TConfigParam::TType::TEnum || m_enums.contains(value)){
            if(ok != nullptr) *ok = false;
            return;
        }
        m_enums.append(value);
        if(ok != nullptr) *ok = true;
    }

    void removeEnumValue(const QString &value, bool *ok = nullptr){
        qsizetype noOfRemoved = m_enums.removeAll(value);
        if(ok != nullptr && noOfRemoved > 0){
            *ok = true;
        } else if(ok != nullptr){
            *ok = false;
        }
    }

    QList<TConfigParam> & getSubParams(){
        return m_subParams;
    }

    void addSubParam(const TConfigParam &param, bool *ok = nullptr){
        if(m_subParams.contains(param)){
            if(ok != nullptr) *ok = false;
            return;
        }
        m_subParams.append(param);
        if(ok != nullptr) *ok = true;
    }

    void removeSubParam(const QString &name, bool *ok = nullptr){
        qsizetype noOfRemoved = m_subParams.removeAll(name);
        if(ok != nullptr && noOfRemoved > 0){
            *ok = true;
        } else if(ok != nullptr){
            *ok = false;
        }
    }

    TConfigParam * getSubParamByName(const QString &name, bool *ok = nullptr){
        int index = this->getSubParams().indexOf(name);
        if(index < 0){
            if(ok != nullptr) *ok = false;
            return nullptr;
        } else {
            if(ok != nullptr) *ok = true;
            return &(this->getSubParams()[index]);
        }
    }

protected:
    QString m_name;
    QString m_defaultValue;
    QString m_value;
    TType m_type;
    QString m_hint;
    TState m_state;
    QString m_stateMessage;
    bool m_readonly;    
    QList<QString> m_enums;
    QList<TConfigParam> m_subParams;

};


#endif // TCONFIGPARAM_H
