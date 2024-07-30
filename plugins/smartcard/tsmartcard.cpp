#include "tsmartcard.h"

TSmartCard::TSmartCard(): m_readers(), m_preInitParams(), m_postInitParams() {
    m_preInitParams  = TConfigParam("Auto-detect", "true", TConfigParam::TType::TBool, "Automatically detect smart card readers", true);

    //m_timer = new QTimer(this);
    //connect(m_timer, &QTimer::timeout, this, QOverload<>::of(&TSmartCard::pingCards));

}

TSmartCard::~TSmartCard() {

    (*this).TSmartCard::deInit();    
    //delete m_timer;

}

QString TSmartCard::getName() const {
    return QString("Smart card");
}

QString TSmartCard::getInfo() const {
    return QString("Provides access to local smart card readers.");
}


TConfigParam TSmartCard::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TSmartCard::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TSmartCard::init(bool *ok) {

    //QMutexLocker locker(&m_mx);

    if(m_preInitParams.getName() == "Auto-detect" && m_preInitParams.getValue() == "true") { // if auto-detect is enabled

        DWORD ret;
        LPSTR mszReaders;
        DWORD pcchReaders = SCARD_AUTOALLOCATE;
        SCARDCONTEXT context;

        SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &context);
        ret = SCardListReadersA(context, NULL, (LPSTR)&mszReaders, &pcchReaders);

        if (ret == SCARD_S_SUCCESS) {

            LPSTR mszReadersTmp = mszReaders;

            // mszReaders is a Multi-$tring, e.g. "abc\0def\0\0"
            while (*mszReadersTmp != 0) {

                QString readerName(mszReadersTmp);
                m_readers.append(new TSmartCardDevice(readerName));

                // move to the next string
                mszReadersTmp += strlen(mszReadersTmp) + 1;

            }

        } else if(ret == SCARD_E_NO_READERS_AVAILABLE) {
            // No SmartCard reader found

        } else {
            qWarning("Error querying Smart Card readers");
            if(ok != nullptr) *ok = false;
        }

        SCardFreeMemory(context, mszReaders);
        SCardReleaseContext(context);

    }

    //m_timer->start(2000);

    if(ok != nullptr) *ok = true;

}

void TSmartCard::deInit(bool *ok) {

    //m_timer->stop();

    // TODO wait for pings to finish!!! mutex?
    //QMutexLocker locker(&m_mx);

    qDeleteAll(m_readers.begin(), m_readers.end());
    m_readers.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TSmartCard::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TSmartCard::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TSmartCard::addIODevice(QString name, QString info, bool *ok) {

    //QMutexLocker locker(&m_mx);

    TIODevice * ret = new TSmartCardDevice(name, info);
    m_readers.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TScope * TSmartCard::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}

bool TSmartCard::canAddIODevice() {
    return true;
}

bool TSmartCard::canAddScope() {
    return false;
}

QList<TIODevice *> TSmartCard::getIODevices() {
    //QMutexLocker locker(&m_mx);
    return m_readers;
}

QList<TScope *> TSmartCard::getScopes() {
    return QList<TScope *>();
}

/*void TSmartCard::pingCards(){

    //QMutexLocker locker(&m_mx);


}*/
