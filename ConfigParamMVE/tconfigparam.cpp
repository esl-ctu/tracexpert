#include "tconfigparam.h"

QDataStream & operator<<(QDataStream &out, const TConfigParam &x) {
    out << QString(TCONFIGPARAMVER);
    out << x.m_name;
    out << x.m_defaultValue;
    out << x.m_value;
    out << x.m_type;
    out << x.m_hint;
    out << x.m_warning;
    out << x.m_warningVal;
    out << x.m_enums;
    out << x.m_subParams;
    return out;
}

QDataStream & operator>>(QDataStream &in, TConfigParam &x) {
    QString verString;
    in >> verString;
    if(Q_LIKELY(verString == QString(TCONFIGPARAMVER))){
        in >> x.m_name;
        in >> x.m_defaultValue;
        in >> x.m_value;
        in >> x.m_type;
        in >> x.m_hint;
        in >> x.m_warning;
        in >> x.m_warningVal;
        in >> x.m_enums;
        in >> x.m_subParams;
    } else {
        qCritical("Failed deserializing TConfigParam: Wrong version or wrong data.");
    }
    return in;
}

TConfigParam::TConfigParam() {}

TConfigParam::TConfigParam(const QString &name, const QString &defaultValue, enum TType type, const QString &hint): m_name(name), m_defaultValue(defaultValue), m_value(defaultValue), m_type(type), m_hint(hint), m_warning(false), m_warningVal(), m_enums(), m_subParams() {

}

TConfigParam::TConfigParam(const TConfigParam &x): m_name(x.m_name), m_defaultValue(x.m_defaultValue), m_value(x.m_value), m_type(x.m_type), m_hint(x.m_hint), m_warning(x.m_warning), m_warningVal(x.m_warningVal), m_enums(x.m_enums), m_subParams(x.m_subParams) {

}

TConfigParam & TConfigParam::operator=(const TConfigParam &x){
    if(&x != this){
        m_name = x.m_name;
        m_defaultValue = x.m_defaultValue;
        m_value = x.m_value;
        m_type = x.m_type;
        m_hint = x.m_hint;
        m_warning = x.m_warning;
        m_warningVal = x.m_warningVal;
        m_enums = x.m_enums;
        m_subParams = x.m_subParams;
    }
    return *this;
}

bool TConfigParam::operator==(const TConfigParam &x) const {
    return (m_name == x.m_name);
}

bool TConfigParam::operator==(const QString &x) const {
    return (m_name == x);
}

const QString & TConfigParam::getName() const {
    return m_name;
}
const QString & TConfigParam::getValue() const {
    return m_value;
}
const QString & TConfigParam::getDefaultValue() const {
    return m_defaultValue;
}
const enum TConfigParam::TType TConfigParam::getType() const {
    return m_type;
}
const QString & TConfigParam::getHint() const {
    return m_hint;
}

void TConfigParam::setWarning(const QString &value){
    m_warning = true;
    m_warningVal = value;
}

void TConfigParam::resetWarning(){
    m_warning = false;
}

bool TConfigParam::isWarning() const {
    return m_warning;
}

const QString & TConfigParam::getWarning() const {
    return m_warningVal;
}

const QString & TConfigParam::setValue(const QString &value, bool *ok){

    bool iok=true;

    switch(m_type) {
        case TType::TString:
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
        case TType::TDouble:
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

    if(iok == true){
        m_value = value;
    }

    if(ok != nullptr) *ok = iok;

    return m_value;
}

qint32 TConfigParam::setValue(qint32 value, bool *ok){
    bool iok;
    int ret = this->setValue(QString::number(value), ok).toInt(&iok);
    if(ok != nullptr){
        *ok = *ok && iok;
    }
    return ret;
}

quint32 TConfigParam::setValue(quint32 value, bool *ok){
    bool iok;
    uint ret = this->setValue(QString::number(value), ok).toUInt(&iok);
    if(ok != nullptr){
        *ok = *ok && iok;
    }
    return ret;
}
qint16 TConfigParam::setValue(qint16 value, bool *ok){
    bool iok;
    short ret = this->setValue(QString::number(value), ok).toShort(&iok);
    if(ok != nullptr){
        *ok = *ok && iok;
    }
    return ret;
}
quint16 TConfigParam::setValue(quint16 value, bool *ok){
    bool iok;
    ushort ret = this->setValue(QString::number(value), ok).toUShort(&iok);
    if(ok != nullptr){
        *ok = *ok && iok;
    }
    return ret;
}
qint64 TConfigParam::setValue(qint64 value, bool *ok){
    bool iok;
    qlonglong ret = this->setValue(QString::number(value), ok).toLongLong(&iok);
    if(ok != nullptr){
        *ok = *ok && iok;
    }
    return ret;
}
quint64 TConfigParam::setValue(quint64 value, bool *ok){
    bool iok;
    qulonglong ret = this->setValue(QString::number(value), ok).toULongLong(&iok);
    if(ok != nullptr){
        *ok = *ok && iok;
    }
    return ret;
}
double TConfigParam::setValue(double value, bool *ok){
    bool iok;
    double ret = this->setValue(QString::number(value), ok).toDouble(&iok);
    if(ok != nullptr){
        *ok = *ok && iok;
    }
    return ret;
}
bool TConfigParam::setBool(bool value, bool *ok){
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

void TConfigParam::resetDefaultValue(){
    m_value = m_defaultValue;
}

const QList<QString> & TConfigParam::getEnumValues() const{
    return m_enums;
}

void TConfigParam::addEnumValue(const QString &value, bool *ok){
    if(m_type != TConfigParam::TType::TEnum || m_enums.contains(value)){
        if(ok != nullptr) *ok = false;
        return;
    }
    m_enums.append(value);
    if(ok != nullptr) *ok = true;
}
void TConfigParam::removeEnumValue(const QString &value, bool *ok){
    qsizetype noOfRemoved = m_enums.removeAll(value);
    if(ok != nullptr && noOfRemoved > 0){
        *ok = true;
    } else if(ok != nullptr){
        *ok = false;
    }
}

QList<TConfigParam> & TConfigParam::getSubParams() {
    return m_subParams;
}

void TConfigParam::addSubParam(const TConfigParam &param, bool *ok) {
    if(m_subParams.contains(param)){
        if(ok != nullptr) *ok = false;
        return;
    }
    m_subParams.append(param);
    if(ok != nullptr) *ok = true;
}

void TConfigParam::removeSubParam(const QString &name, bool *ok){
    qsizetype noOfRemoved = m_subParams.removeAll(name);
    if(ok != nullptr && noOfRemoved > 0){
        *ok = true;
    } else if(ok != nullptr){
        *ok = false;
    }
}

TConfigParam * TConfigParam::getSubParamByName(const QString &name, bool *ok){
    int index = this->getSubParams().indexOf(name);
    if(index < 0){
        if(ok != nullptr) *ok = false;
        return nullptr;
    } else {
        if(ok != nullptr) *ok = true;
        return &(this->getSubParams()[index]);
    }
}
