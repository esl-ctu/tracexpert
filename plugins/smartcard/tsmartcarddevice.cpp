#include "tsmartcarddevice.h"

TSmartCardDevice::TSmartCardDevice(QString & name, QString & info):
    m_createdManually(true), m_name(name), m_info(info), m_initialized(false), m_inputCreateAPDU(false), m_outputParseAPDU(false)
{

    // Pre-init parameters with system location (pre-filled with port name)
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("Reader ID string", m_name, TConfigParam::TType::TString, "Reader ID string as seen by PC/SC API", false));
    m_preInitParams.addSubParam(TConfigParam("Protocol", "T=1", TConfigParam::TType::TString, "Only T=1 protocol is currently supported", true));

    m_APDUHeader[0] = 0;
    m_APDUHeader[1] = 0;
    m_APDUHeader[2] = 0;
    m_APDUHeader[3] = 0;
    m_APDUTrailer = 0;

}

TSmartCardDevice::TSmartCardDevice(QString & mszReader):
    m_createdManually(false), m_name(mszReader), m_info("Automatically detected"), m_initialized(false), m_inputCreateAPDU(false), m_outputParseAPDU(false)
{

    // Pre-init parameters are all read-only for automatically detected devices
    m_preInitParams = TConfigParam(m_name + " pre-init configuration", "", TConfigParam::TType::TDummy, "");
    m_preInitParams.addSubParam(TConfigParam("Reader ID string", m_name, TConfigParam::TType::TString, "Reader ID string as seen by PC/SC API", true));
    m_preInitParams.addSubParam(TConfigParam("Protocol", "T=1", TConfigParam::TType::TString, "Only T=1 protocol is currently supported", true));

    m_APDUHeader[0] = 0;
    m_APDUHeader[1] = 0;
    m_APDUHeader[2] = 0;
    m_APDUHeader[3] = 0;
    m_APDUTrailer = 0;
}

TSmartCardDevice::~TSmartCardDevice() {
    (*this).TSmartCardDevice::deInit();
}

QString TSmartCardDevice::getName() const {
    return m_name;
}

QString TSmartCardDevice::getInfo() const {
    return m_info;
}


TConfigParam TSmartCardDevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TSmartCardDevice::setPreInitParams(TConfigParam params) {

    if(m_initialized){
        m_preInitParams.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
    } else {
        m_preInitParams = params;
        m_preInitParams.resetState();
    }
    return m_preInitParams;
}

void TSmartCardDevice::init(bool *ok) {

    bool iok;

    TConfigParam * readerStrParam = m_preInitParams.getSubParamByName("Reader ID string", &iok);
    if(!iok){
        qWarning("System location parameter not found in the pre-init config.");
        if(ok != nullptr) *ok = false;
        return;
    }

    SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &m_context);

    DWORD ret;
    DWORD protocol;
    QByteArray szReaderQB = (readerStrParam->getValue()).toLatin1();
    LPCSTR szReader = szReaderQB.constData();

    ret = SCardConnectA(m_context, szReader, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T1, &m_card, &protocol);

    if (ret != SCARD_S_SUCCESS) {
        SCardReleaseContext(m_context);
        qWarning("Failed to connect to the card.");
        if(ok != nullptr) *ok = false;
        return;
    } else if(protocol != SCARD_PROTOCOL_T1){
        SCardReleaseContext(m_context);
        qWarning("Failed to negotiate the T1 protocol.");
        if(ok != nullptr) *ok = false;
        return;
    }

    ret = SCardBeginTransaction(m_card);

    if (ret != SCARD_S_SUCCESS) {
        SCardDisconnect(m_card, SCARD_LEAVE_CARD);
        SCardReleaseContext(m_context);
        qWarning("Failed to begin a card transaction.");
        if(ok != nullptr) *ok = false;
        return;
    }

    BYTE atr[32];
    DWORD atrLen = 32;
    DWORD state;

    ret = SCardStatus(m_card, NULL, NULL, &state, NULL, atr, &atrLen);

    if (ret != SCARD_S_SUCCESS) {
        qWarning("Failed to check the card status.");
        if(ok != nullptr) *ok = false;
        return;
    }

    switch (state) {
        case SCARD_ABSENT:
            qWarning("There is no card in the reader.");
            if(ok != nullptr) *ok = false;
            return;
            break;
        case SCARD_PRESENT:
            qWarning("There is a card in the reader, but it has not been moved into position for use.");
            if(ok != nullptr) *ok = false;
            return;
            break;
        case SCARD_SWALLOWED:
            qWarning("There is a card in the reader in position for use. The card is not powered.");
            if(ok != nullptr) *ok = false;
            return;
            break;
        case SCARD_POWERED:
            qWarning("Power is being provided to the card, but the reader driver is unaware of the mode of the card.");
            if(ok != nullptr) *ok = false;
            return;
            break;
        case SCARD_NEGOTIABLE:
            qWarning("The card has been reset and is awaiting PTS negotiation.");
            if(ok != nullptr) *ok = false;
            return;
            break;
        case SCARD_SPECIFIC:
            //printf("The card has been reset and specific communication protocols have been established.");
            break;
        default:
            qWarning("Unknown or unexpected card state.");
            if(ok != nullptr) *ok = false;
            return;
            break;
    }

    m_initialized = true;
    _createPostInitParams();


}

void TSmartCardDevice::_createPostInitParams(){

    m_postInitParams = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");

    TConfigParam inputParam = TConfigParam("Input format", "Command APDU", TConfigParam::TType::TEnum, "When Auto APDU construction is set, the APDU is constructed according to pre-set parameters and the length is computed.");
    inputParam.addEnumValue("Data only (pre-defined command APDU)");
    inputParam.addEnumValue("Command APDU");
    m_postInitParams.addSubParam(inputParam);

    TConfigParam outputParam = TConfigParam("Output format", "Response APDU", TConfigParam::TType::TEnum, "Enables/disables SW1-SW2 stripping from the response APDU");
    outputParam.addEnumValue("Data only (SW1-SW2 stripping)");
    outputParam.addEnumValue("Response APDU");
    m_postInitParams.addSubParam(outputParam);


    TConfigParam apduParams = TConfigParam("Data only: pre-defined command APDU", "", TConfigParam::TType::TDummy, "Only used when Input = Data only");
    apduParams.addSubParam(TConfigParam("CLA (1 byte hex)", "00", TConfigParam::TType::TString, "Instruction class - indicates the type of command, e.g., interindustry or proprietary"));
    apduParams.addSubParam(TConfigParam("INS (1 byte hex)", "00", TConfigParam::TType::TString, "Instruction code - indicates the specific command, e.g., select, write data"));
    apduParams.addSubParam(TConfigParam("P1 (1 byte hex)", "00", TConfigParam::TType::TString, "Instruction parameters for the command, e.g., offset into file at which to write the data"));
    apduParams.addSubParam(TConfigParam("P2 (1 byte hex)", "00", TConfigParam::TType::TString, "Instruction parameters for the command, e.g., offset into file at which to write the data"));
    apduParams.addSubParam(TConfigParam("Lc (1 byte hex)", "auto, max FF", TConfigParam::TType::TString, "Encodes the number (Nc) of bytes of command data to follow", true));
    apduParams.addSubParam(TConfigParam("Le (1 byte hex)", "00", TConfigParam::TType::TString, "Encodes the maximum number (Ne) of response bytes expected"));
    apduParams.setState(TConfigParam::TState::TWarning, "Not used when input format is Command APDU");
    m_postInitParams.addSubParam(apduParams);

}

bool TSmartCardDevice::_validatePostInitParamsStructure(TConfigParam & params) {

    // Only checks the structure of parameters. Values are validated later during init. Enum values are checked during their setting by the TConfigParam.

    bool iok = false;

    params.getSubParamByName("Input format", &iok);
    if(!iok) return false;

    params.getSubParamByName("Output format", &iok);
    if(!iok) return false;

    TConfigParam * apduParams = params.getSubParamByName("Data only: pre-defined command APDU", &iok);
    if(!iok) return false;

    apduParams->getSubParamByName("CLA (1 byte hex)", &iok);
    if(!iok) return false;

    apduParams->getSubParamByName("INS (1 byte hex)", &iok);
    if(!iok) return false;

    apduParams->getSubParamByName("P1 (1 byte hex)", &iok);
    if(!iok) return false;

    apduParams->getSubParamByName("P2 (1 byte hex)", &iok);
    if(!iok) return false;

    apduParams->getSubParamByName("Le (1 byte hex)", &iok);
    if(!iok) return false;

    return true;

}

void TSmartCardDevice::deInit(bool *ok) {

    m_initialized = false;
    SCardEndTransaction(m_card, SCARD_LEAVE_CARD);
    SCardDisconnect(m_card, SCARD_LEAVE_CARD);
    SCardReleaseContext(m_context);    
    if(ok != nullptr) *ok = true;

}

TConfigParam TSmartCardDevice::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TSmartCardDevice::setPostInitParams(TConfigParam params) {

    if(!_validatePostInitParamsStructure(params)){
        qCritical("Wrong structure of the post-init params for TSmartCardDevice");
        return m_postInitParams;
    }

    m_postInitParams = params;
    m_postInitParams.resetState(true);

    TConfigParam * inputParam = m_postInitParams.getSubParamByName("Input format");
    QString inputVal = inputParam->getValue();
    TConfigParam * outputParam = m_postInitParams.getSubParamByName("Output format");
    QString outputVal = outputParam->getValue();

    TConfigParam * apduParams = m_postInitParams.getSubParamByName("Data only: pre-defined command APDU");

    TConfigParam * claParam = apduParams->getSubParamByName("CLA (1 byte hex)");
    QString claVal = claParam->getValue();
    TConfigParam * insParam = apduParams->getSubParamByName("INS (1 byte hex)");
    QString insVal = insParam->getValue();
    TConfigParam * p1Param = apduParams->getSubParamByName("P1 (1 byte hex)");
    QString p1Val = p1Param->getValue();
    TConfigParam * p2Param = apduParams->getSubParamByName("P2 (1 byte hex)");
    QString p2Val = p2Param->getValue();
    TConfigParam * leParam = apduParams->getSubParamByName("Le (1 byte hex)");
    QString leVal = leParam->getValue();

    bool iok;

    if(inputVal == "Data only (pre-defined command APDU)"){
        m_inputCreateAPDU = true;

        uint claInt = claVal.toUInt(&iok, 16);
        if(iok != true) {
            claInt = 0;
            claParam->setState(TConfigParam::TState::TError, "Invalid hexadecimal value, 00 used instead");
            //claParam->setValue("00");
        }
        if(claInt > 255) {
            claInt = 255;
            claParam->setState(TConfigParam::TState::TError, "Value greater than FF, FF used instead");
            //claParam->setValue("FF");
        }

        uint insInt = insVal.toUInt(&iok, 16);
        if(iok != true) {
            insInt = 0;
            insParam->setState(TConfigParam::TState::TError, "Invalid hexadecimal value, 00 used instead");
            //insParam->setValue("00");
        }
        if(insInt > 255) {
            insInt = 255;
            insParam->setState(TConfigParam::TState::TError, "Value greater than FF, FF used instead");
            //insParam->setValue("FF");
        }

        uint p1Int = p1Val.toUInt(&iok, 16);
        if(iok != true) {
            p1Int = 0;
            p1Param->setState(TConfigParam::TState::TError, "Invalid hexadecimal value, 00 used instead");
            //p1Param->setValue("00");
        }
        if(p1Int > 255) {
            p1Int = 255;
            p1Param->setState(TConfigParam::TState::TError, "Value greater than FF, FF used instead");
            //p1Param->setValue("FF");
        }

        uint p2Int = p2Val.toUInt(&iok, 16);
        if(iok != true) {
            p2Int = 0;
            p2Param->setState(TConfigParam::TState::TError, "Invalid hexadecimal value, 00 used instead");
            //p2Param->setValue("00");
        }
        if(p2Int > 255) {
            p2Int = 255;
            p2Param->setState(TConfigParam::TState::TError, "Value greater than FF, FF used instead");
            //p2Param->setValue("FF");
        }

        // LE NOVE HEX! PREDELAT!
        uint leInt = leVal.toUInt(&iok, 16);
        if(iok != true) {
            leInt = 0;
            leParam->setState(TConfigParam::TState::TError, "Invalid hexadecimal value, 00 used instead");
            //leParam->setValue("00");
        }
        if(leInt > 255) {
            leInt = 255;
            leParam->setState(TConfigParam::TState::TError, "Value greater than FF, FF used instead");
            //leParam->setValue("FF");
        }

        m_APDUHeader[0] = claInt;
        m_APDUHeader[1] = insInt;
        m_APDUHeader[2] = p1Int;
        m_APDUHeader[3] = p2Int;
        m_APDUTrailer = leInt;

    } else if (inputVal == "Command APDU") {
        apduParams->setState(TConfigParam::TState::TWarning, "Not used when input format is Command APDU");
        m_inputCreateAPDU = false;

    } else {
        qCritical("Unexpected Input format param value");
    }

    if(outputVal == "Data only (SW1-SW2 stripping)"){
        m_outputParseAPDU = true;

    } else if (outputVal == "Response APDU"){
        m_outputParseAPDU = false;

    } else {
        qCritical("Unexpected Output format param value");
    }

    return m_postInitParams;

}

size_t TSmartCardDevice::writeData(const uint8_t * buffer, size_t len){

    if(m_inputCreateAPDU == true && len > 255){
        qWarning("Too much data to write using Data-only input format. Use raw Command APDU.");
        return 0;
    }

    DWORD ret;

    DWORD sendLen;
    QByteArray sendBuf;

    DWORD recvLen = 65537;
    QByteArray recvBuf(65537 + 1, 0);

    // Prepare output buffer
    if(m_inputCreateAPDU == true){ // create APDU
        sendBuf = QByteArray(len + 6 + 1, 0);
        sendLen = len + 6;

        unsigned char * sendBufPtr = (unsigned char *) sendBuf.data();

        memcpy(sendBufPtr, m_APDUHeader, 4);
        *(sendBufPtr + 4) = (unsigned char) len;
        memcpy(sendBufPtr + 5, buffer, len);
        *(sendBufPtr + len + 5) = m_APDUTrailer;

    } else { // send raw data
        sendBuf = QByteArray((const char *) buffer, len);
        sendLen = len;
    }

    // Transmit data
    ret = SCardTransmit(m_card, SCARD_PCI_T1, (const unsigned char *) sendBuf.constData(), sendLen, NULL, (unsigned char *) recvBuf.data(), &recvLen);

    if(ret != SCARD_S_SUCCESS){ //(ret == SCARD_W_RESET_CARD){ // the transaction has probably expired, reconnect

        DWORD pdwActiveProtocol;

        ret = SCardReconnect(m_card, SCARD_SHARE_EXCLUSIVE, SCARD_PROTOCOL_T1, SCARD_LEAVE_CARD, &pdwActiveProtocol);

        if (ret != SCARD_S_SUCCESS) {
            qWarning("Failed to reconnect the card");
            return 0;
        }

        ret = SCardBeginTransaction(m_card);

        if (ret != SCARD_S_SUCCESS) {
            qWarning("Failed to begin transaction");
            return 0;
        }

        recvLen = 65537;
        ret = SCardTransmit(m_card, SCARD_PCI_T1, (const unsigned char *) sendBuf.constData(), sendLen, NULL, (unsigned char *) recvBuf.data(), &recvLen);

        if(ret != SCARD_S_SUCCESS){
            qWarning("Failed sending the data");
            qWarning(QString::number(ret).toLatin1());
            return 0;
        }

    }

    if(m_outputParseAPDU == true){
        recvLen -= 2; // remove SW1-SW2 if set
    }

    recvBuf.resize(recvLen);
    m_recQueue.enqueue(recvBuf);

    return 0;

}

size_t TSmartCardDevice::readData(uint8_t * buffer, size_t len) {

    size_t readLen = 0;

    while(m_recQueue.empty() != true && readLen < len) {

        if((m_recQueue.head()).size() <= (len - readLen)){ // the whole bytearray fits the buffer
            memcpy(buffer + readLen, (m_recQueue.head()).constData(), (m_recQueue.head()).size()); // copy the whole bytearray to the buffer
            readLen += (m_recQueue.head()).size();
            m_recQueue.dequeue();
        } else { // only a part of the bytearray fits the buffer
            memcpy(buffer + readLen, (m_recQueue.head()).constData(), len - readLen); // copy a part of the bytearray to the buffer
            (m_recQueue.head()).remove(0, len - readLen); // remove the read part from the buffer
            readLen += len - readLen;
        }

    }

    return readLen;

}

std::optional<size_t> TSmartCardDevice::availableBytes(){

    qsizetype totalBytes = 0;

    for (const QByteArray &chunk : m_recQueue) {
        totalBytes += chunk.size();
    }

    return totalBytes;

}

