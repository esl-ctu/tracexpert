#include "taesengine.h"

#include <QtGlobal>

#include "tcipheraction.h"
#include "tcipherinputstream.h"
#include "tcipheroutputstream.h"
#include "taes.hpp"

TAESEngine::TAESEngine(): m_operation(0), m_keysizeB(16), m_position(0), m_breakpoint(5), m_breakpointN(1), m_outputRestrict(0), m_outputRestrictM(0) {

    m_preInitParams = TConfigParam("AES configuration", "", TConfigParam::TType::TDummy, "");

    TConfigParam keyType = TConfigParam("Key length", "128 bit", TConfigParam::TType::TEnum, "Size of the AES key");
    keyType.addEnumValue("128 bit");
    keyType.addEnumValue("192 bit");
    keyType.addEnumValue("256 bit");
    m_preInitParams.addSubParam(keyType);

    TConfigParam operationType = TConfigParam("Operation", "Encryption", TConfigParam::TType::TEnum, "AES operation");
    operationType.addEnumValue("Encryption");
    operationType.addEnumValue("Decryption");
    m_preInitParams.addSubParam(operationType);

}

TAESEngine::~TAESEngine() {
    (*this).TAESEngine::deInit();
}

QString TAESEngine::getName() const {
    return QString("AES");
}

QString TAESEngine::getInfo() const {
    return QString("Provides AES intermediate values during encryption/decryption.");
}


TConfigParam TAESEngine::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TAESEngine::setPreInitParams(TConfigParam params) {

    bool iok;

    m_preInitParams = params;
    m_preInitParams.resetState(true);

    TConfigParam * operationParam = m_preInitParams.getSubParamByName("Operation", &iok);
    if(!iok) {
        qCritical("AES operation parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    if(operationParam->getValue() == "Encryption"){
        m_operation = 0;
    } else {
        m_operation = 1;
    }

    TConfigParam * keyParam = m_preInitParams.getSubParamByName("Key length", &iok);
    if(!iok) {
        qCritical("AES key size parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    if(keyParam->getValue() == "128 bit"){
        m_keysizeB = 16;
    } else if (keyParam->getValue() == "192 bit"){
        m_keysizeB = 24;
    } else {
        m_keysizeB = 32;
    }

    return m_preInitParams;

}

void TAESEngine::init(bool *ok) {

    TConfigParam postInitParams;
    if(m_operation == 0){
        if(m_keysizeB == 16){
            postInitParams = TConfigParam("AES-128 encryption output config", "", TConfigParam::TType::TDummy, "");
        } else if(m_keysizeB == 24) {
            postInitParams = TConfigParam("AES-192 encryption output config", "", TConfigParam::TType::TDummy, "");
        } else {
            postInitParams = TConfigParam("AES-256 encryption output config", "", TConfigParam::TType::TDummy, "");
        }
    } else {
        if(m_keysizeB == 16){
            postInitParams = TConfigParam("AES-128 decryption output config", "", TConfigParam::TType::TDummy, "");
        } else if(m_keysizeB == 24) {
            postInitParams = TConfigParam("AES-192 decryption output config", "", TConfigParam::TType::TDummy, "");
        } else {
            postInitParams = TConfigParam("AES-256 decryption output config", "", TConfigParam::TType::TDummy, "");
        }
    }

    if(m_operation == 0){ // Encryption
        TConfigParam interValPar = TConfigParam("Intermediate value", "Ciphertext", TConfigParam::TType::TEnum, "Intermediate value to return");
        interValPar.addEnumValue("Plaintext");
        interValPar.addEnumValue("After Nth AddRoundKey");
        interValPar.addEnumValue("After Nth SubBytes");
        interValPar.addEnumValue("After Nth ShiftRows");
        interValPar.addEnumValue("After Nth MixColumns");
        interValPar.addEnumValue("Ciphertext");
        interValPar.addSubParam(TConfigParam("N", "1", TConfigParam::TType::TUShort, "Value N, greater than 0"));
        postInitParams.addSubParam(interValPar);
    } else { // Decryption
        TConfigParam interValPar = TConfigParam("Intermediate value", "Plaintext", TConfigParam::TType::TEnum, "Intermediate value to return");
        interValPar.addEnumValue("Ciphertext");
        interValPar.addEnumValue("After Nth AddRoundKey");
        interValPar.addEnumValue("After Nth InvShiftRows");
        interValPar.addEnumValue("After Nth InvSubBytes");
        interValPar.addEnumValue("After Nth InvMixColumns");
        interValPar.addEnumValue("Plaintext");
        interValPar.addSubParam(TConfigParam("N", "1", TConfigParam::TType::TUShort, "Value N, greater than 0"));
        postInitParams.addSubParam(interValPar);
    }

    TConfigParam outPar = TConfigParam("Output", "Whole block", TConfigParam::TType::TEnum, "Stream returns either 16 bytes for Whole block, or a byte when restricted to byte or bit");
    outPar.addEnumValue("Whole block");
    outPar.addEnumValue("Mth byte (leftmost byte = 0)");
    outPar.addEnumValue("Mth bit (rightmost bit = 0)");
    outPar.addSubParam(TConfigParam("M", "0", TConfigParam::TType::TUShort, "Non-negative value M lesser than 16 or 128 respectively"));
    postInitParams.addSubParam(outPar);

    m_postInitParams = postInitParams;

    m_analActions.append(new TCipherAction((m_operation==0) ? "Encrypt input data (+ flush streams)" : "Decrypt input data (+ flush streams)", "", [=](){ computeIntermediates(); }));
    m_analActions.append(new TCipherAction("Load cipher key (+ flush streams)", "", [=](){ loadKey(); }));
    m_analActions.append(new TCipherAction("Reset (delete all data)", "", [=](){ reset(); }));


    m_analOutputStreams.append(new TCipherOutputStream((m_operation == 0) ? "Plaintext" : "Ciphertext", "Stream of input data", [=](const uint8_t * buffer, size_t length){ return addData(buffer, length); }));
    m_analOutputStreams.append(new TCipherOutputStream("Cipher key", "Stream of input data", [=](const uint8_t * buffer, size_t length){ return addKeyData(buffer, length); }));

    m_analInputStreams.append(new TCipherInputStream("Intermediate values", "Stream of output data as selected in configuration", [=](uint8_t * buffer, size_t length){ return getIntermediates(buffer, length); }));


    if (ok != nullptr) *ok = true;

}

void TAESEngine::deInit(bool *ok) {

    for (int i = 0; i < m_analActions.length(); i++) {
        delete m_analActions[i];
    }
    m_analActions.clear();

    for (int i = 0; i < m_analInputStreams.length(); i++) {
        delete m_analInputStreams[i];
    }
    m_analInputStreams.clear();

    for (int i = 0; i < m_analOutputStreams.length(); i++) {
        delete m_analOutputStreams[i];
    }
    m_analOutputStreams.clear();

    m_data.clear();
    m_keyData.clear();
    m_intermediates.clear();
    m_position = 0;

    if (ok != nullptr) *ok = true;
}

TConfigParam TAESEngine::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TAESEngine::setPostInitParams(TConfigParam params) {

    m_postInitParams = params; // read the parameters
    m_postInitParams.resetState(true); // reset all warnings and errors

    bool iok;

    TConfigParam * interValPar = m_postInitParams.getSubParamByName("Intermediate value", &iok);
    if(!iok){
        qCritical("Intermediate value param not found in the post-init params");
    }

    TConfigParam * NValPar = interValPar->getSubParamByName("N", &iok);
    if(!iok){
        qCritical("N value param not found in the post-init params");
    }

    TConfigParam * outValPar = m_postInitParams.getSubParamByName("Output", &iok);
    if(!iok){
        qCritical("Output settings param not found in the post-init params");
    }

    TConfigParam * MValPar = outValPar->getSubParamByName("M", &iok);
    if(!iok){
        qCritical("M value param not found in the post-init params");
    }

    QString interVal = interValPar->getValue();
    QString NVal = NValPar->getValue();
    QString outVal = outValPar->getValue();
    QString MVal = MValPar->getValue();

    if(m_operation == 0){ // Encryption
        if(interVal == "Plaintext") {
            m_breakpoint = 0;
        } else if (interVal == "After Nth AddRoundKey") {
            m_breakpoint = 1;
        } else if (interVal == "After Nth SubBytes") {
            m_breakpoint = 2;
        } else if (interVal == "After Nth ShiftRows") {
            m_breakpoint = 3;
        } else if (interVal == "After Nth MixColumns") {
            m_breakpoint = 4;
        } else if (interVal == "Ciphertext") {
            m_breakpoint = 5;
        } else {
            interValPar->setState(TConfigParam::TState::TError, "Invalid intermediate value");
        }
    } else { // Decryption
        if(interVal == "Ciphertext") {
            m_breakpoint = 0;
        } else if (interVal == "After Nth AddRoundKey") {
            m_breakpoint = 1;
        } else if (interVal == "After Nth InvShiftRows") {
            m_breakpoint = 2;
        } else if (interVal == "After Nth InvSubBytes") {
            m_breakpoint = 3;
        } else if (interVal == "After Nth InvMixColumns") {
            m_breakpoint = 4;
        } else if (interVal == "Plaintext") {
            m_breakpoint = 5;
        } else {
            interValPar->setState(TConfigParam::TState::TError, "Invalid intermediate value");
        }
    }

    size_t NValUInt = NVal.toUShort();
    if(NValUInt < 1){
        NValPar->setState(TConfigParam::TState::TError, "Invalid N value");
    } else {
        m_breakpointN = NValUInt;
        // TODO validation with respect to selected intermediate value, operation mode and key size
    }

    if(outVal == "Whole block") {
        m_outputRestrict = 0;
    } else if(outVal == "Mth byte (leftmost byte = 0)") {
        m_outputRestrict = 1;
    } else if(outVal == "Mth bit (rightmost bit = 0)") {
        m_outputRestrict = 2;
    } else {
        outValPar->setState(TConfigParam::TState::TError, "Invalid output format");
    }

    size_t MValUInt = MVal.toUShort();
    if(MValUInt < 0){
        MValPar->setState(TConfigParam::TState::TError, "M value must be greater than 0");
    } else if(MValUInt > 15 && m_outputRestrict == 1) {
        MValPar->setState(TConfigParam::TState::TError, "M value must be lower than 16");
    } else if(MValUInt > 127 && m_outputRestrict == 2) {
        MValPar->setState(TConfigParam::TState::TError, "M value must be lower than 128");
    } else {
        m_outputRestrictM = MValUInt;
    }

    return m_postInitParams;
}

QList<TAnalAction *> TAESEngine::getActions() const
{
    return m_analActions;
}

QList<TAnalInputStream *> TAESEngine::getInputDataStreams() const
{
    return m_analInputStreams;
}

QList<TAnalOutputStream *> TAESEngine::getOutputDataStreams() const
{
    return m_analOutputStreams;
}

bool TAESEngine::isBusy() const
{
    return false;
}

size_t TAESEngine::addData(const uint8_t * buffer, size_t length){

    m_data.reserve(m_data.size() + length); // preallocate space

    for(int i = 0; i < length; i++){
        m_data.append(buffer[i]);
    }

    return length;

}

size_t TAESEngine::addKeyData(const uint8_t * buffer, size_t length){

    m_keyData.reserve(m_keyData.size() + length); // preallocate space

    for(int i = 0; i < length; i++){
        m_keyData.append(buffer[i]);
    }

    return length;

}

void TAESEngine::reset() {

    m_data.clear();
    m_keyData.clear();
    m_intermediates.clear();
    m_position = 0;

    // TODO clear key

}

size_t TAESEngine::getIntermediates(uint8_t * buffer, size_t length){

    size_t sent = 0;

    for (sent = 0; m_position < m_intermediates.size() && sent < length; sent++, m_position++) {
        buffer[sent] = m_intermediates[m_position];
    }

    /*if(m_position == m_intermediates.size()){
        m_intermediates.clear();
        m_position = m_intermediates.length();
    }*/

    return sent;

}

void TAESEngine::loadKey(){

    if(m_keyData.length() != m_keysizeB){
        qCritical("Key buffer does not contain a valid amount of bytes (16 B for AES-128, 24 B for AES-192, 32 B for AES-256)");
        return;
    }

    for(int i = 0; i < m_keysizeB; i++){
        m_key[i] = m_keyData[i];
    }

    m_keyData.clear();

}

void TAESEngine::computeIntermediates(){

    if(m_data.length() % 16 != 0){
        qCritical("Plaintext/Ciphertext buffer does not contain a valid amount of bytes (not divisible by 16)");
        return;
    }

    // flush stream
    m_intermediates.clear();
    m_position = m_intermediates.length();

    size_t blocksN = m_data.length() / 16;

    uint8_t AESOut[16];

    m_intermediates.reserve(m_intermediates.size() + m_data.length());

    for(int block = 0; block < blocksN; block++){

        uint8_t * AESIn = &(m_data[block * 16]);

        if(m_operation == 0){

            TAES::EncryptBlock(AESOut, AESIn, m_key, m_keysizeB, m_breakpoint, m_breakpointN);

        } else {

            TAES::DecryptBlock(AESOut, AESIn, m_key, m_keysizeB, m_breakpoint, m_breakpointN);

        }

        if(m_outputRestrict == 0){ // return whole block
            for(int byte = 0; byte < 16; byte++){
                m_intermediates.append(AESOut[byte]);
            }
        } else if(m_outputRestrict == 1) { // Mth byte
            m_intermediates.append(AESOut[m_outputRestrictM]);
        } else { // Mth bit
            uint8_t bitValue = (AESOut[15 - (m_outputRestrictM / 8)] >> (m_outputRestrictM % 8)) & 0x01;
            m_intermediates.append(bitValue);
        }

    }

    m_data.clear();

}
