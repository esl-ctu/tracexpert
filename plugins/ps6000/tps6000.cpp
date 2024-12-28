#include "tps6000.h"

TPS6000::TPS6000(): m_scopes(), m_preInitParams() {
    m_preInitParams  = TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect Picoscope 6000 series scopes available", false);
}

TPS6000::~TPS6000() {
    (*this).TPS6000::deInit();
}

QString TPS6000::getName() const {
    return QString("Picoscope 6000 A/B/C/D series");
}

QString TPS6000::getInfo() const {
    return QString("Provides access to Picoscope 6000 A/B/C/D series oscilloscopes");
}


TConfigParam TPS6000::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TPS6000::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TPS6000::init(bool *ok) {
    bool iok = true;
    if(m_preInitParams.getName() == "Auto-detect" && m_preInitParams.getValue() == "true") { // if auto-detect is enabled

        int16_t count;
        std::unique_ptr<int8_t[]> serials(new int8_t[1024]);
        int16_t serials_len = 1024;

        PICO_STATUS status = ps6000EnumerateUnits(&count, serials.get(), &serials_len);

        if(status == PICO_OK){

            QString qserials((char*) serials.get());
            QStringList serialsList = qserials.split(QLatin1Char(','));

            if (count == 0) { // no scope found
                iok = true;
            } else if(serialsList.size() == count) { // scopes found

                for (const QString &serialNo : serialsList) {

                    m_scopes.append(new TPS6000Scope(serialNo, "Automatically detected"));

                }

            } else {
                qWarning("Picoscope auto-detection went wrong. This should never happen.");
                iok = false;
            }

        } else if(status == PICO_NOT_FOUND) { // no scope found
            iok = true;
        } else {
            qWarning("Picoscope auto-detection went wrong. Either the API is busy, or there is a hardware failure.");
            iok = false;
        }
    }
    if(ok != nullptr){
        *ok = iok; // TODO iok setup
    }
}

void TPS6000::deInit(bool *ok) {
    qDeleteAll(m_scopes.begin(), m_scopes.end());
    m_scopes.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TPS6000::getPostInitParams() const {
    return TConfigParam();
}

TConfigParam TPS6000::setPostInitParams(TConfigParam params) {
    return TConfigParam();
}

TIODevice * TPS6000::addIODevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

TScope * TPS6000::addScope(QString name, QString info, bool *ok) {
    TScope * ret = new TPS6000Scope(name, info);
    m_scopes.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TAnalDevice * TPS6000::addAnalDevice(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TPS6000::canAddIODevice() {
    return false;
}

bool TPS6000::canAddScope() {
    return true;
}
bool TPS6000::canAddAnalDevice() {
    return false;
}

QList<TIODevice *> TPS6000::getIODevices() {
    return QList<TIODevice *>();
}

QList<TScope *> TPS6000::getScopes() {
    return m_scopes;
}

QList<TAnalDevice *> TPS6000::getAnalDevices(){
    return QList<TAnalDevice *>();
}
