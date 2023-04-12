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

    TConfigParam();
    TConfigParam(const QString &name, const QString &defaultValue, enum TType type, const QString &hint, bool readonly);
    TConfigParam(const TConfigParam &x);

    TConfigParam & operator=(const TConfigParam &x);

    bool operator==(const TConfigParam &x) const;
    bool operator==(const QString &x) const;

    friend QDataStream & operator<<(QDataStream &out, const TConfigParam &x);
    friend QDataStream & operator>>(QDataStream &in, TConfigParam &x);

    const QString & getName() const;
    const QString & getValue() const;
    const QString & getDefaultValue() const;
    const enum TType getType() const;
    const QString & getHint() const;

    const QString & setValue(const QString &value, bool *ok = nullptr);
    qint32 setValue(qint32 value, bool *ok = nullptr);
    quint32 setValue(quint32 value, bool *ok = nullptr);
    qint16 setValue(qint16 value, bool *ok = nullptr);
    quint16 setValue(quint16 value, bool *ok = nullptr);
    qint64 setValue(qint64 value, bool *ok = nullptr);
    quint64 setValue(quint64 value, bool *ok = nullptr);
    qreal setValue(qreal value, bool *ok = nullptr);
    bool setBool(bool value, bool *ok = nullptr);

    void resetDefaultValue();

    void setState(TState state);
    void setState(TState state, const QString &message);
    void resetState();
    TConfigParam::TState getState() const;
    const QString & getStateMessage() const;

    bool isReadonly() const;

    const QList<QString> & getEnumValues() const;
    void addEnumValue(const QString &value, bool *ok = nullptr);
    void removeEnumValue(const QString &value, bool *ok = nullptr);

    QList<TConfigParam> & getSubParams();
    void addSubParam(const TConfigParam &param, bool *ok = nullptr);
    void removeSubParam(const QString &name, bool *ok = nullptr);
    TConfigParam * getSubParamByName(const QString &name, bool *ok = nullptr);

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
