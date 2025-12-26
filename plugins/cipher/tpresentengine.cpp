// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Petr Socha (initial author)

#include "tpresentengine.h"

#include <QtGlobal>

#include "tcipheraction.h"
#include "tcipherinputstream.h"
#include "tcipheroutputstream.h"
#include "tpresent.hpp"

TPRESENTEngine::TPRESENTEngine(): m_operation(0), m_keysizeB(10), m_position(0), m_breakpoint(4), m_breakpointN(1), m_outputRestrict(0), m_outputRestrictM(0) {

    m_preInitParams = TConfigParam("PRESENT configuration", "", TConfigParam::TType::TDummy, "");

    TConfigParam keyType = TConfigParam("Key length", "80 bit", TConfigParam::TType::TEnum, "Size of the PRESENT key");
    keyType.addEnumValue("80 bit");
    keyType.addEnumValue("128 bit");
    m_preInitParams.addSubParam(keyType);

    TConfigParam operationType = TConfigParam("Operation", "Encryption", TConfigParam::TType::TEnum, "PRESENT operation");
    operationType.addEnumValue("Encryption");
    operationType.addEnumValue("Decryption");
    m_preInitParams.addSubParam(operationType);

}

TPRESENTEngine::~TPRESENTEngine() {
    (*this).TPRESENTEngine::deInit();
}

QString TPRESENTEngine::getName() const {
    return QString("PRESENT");
}

QString TPRESENTEngine::getInfo() const {
    return QString("Provides PRESENT intermediate values during encryption/decryption.");
}


TConfigParam TPRESENTEngine::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TPRESENTEngine::setPreInitParams(TConfigParam params) {

    bool iok;

    m_preInitParams = params;
    m_preInitParams.resetState(true);

    TConfigParam * operationParam = m_preInitParams.getSubParamByName("Operation", &iok);
    if(!iok) {
        qCritical("PRESENT operation parameter not found in the pre-init params");
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
        qCritical("PRESENT key size parameter not found in the pre-init params");
        TConfigParam errorParam;
        errorParam.setState(TConfigParam::TState::TError);
        return errorParam;
    }
    if(keyParam->getValue() == "80 bit"){
        m_keysizeB = 10;
    } else {
        m_keysizeB = 16;
    }

    return m_preInitParams;

}

void TPRESENTEngine::init(bool *ok) {

    TConfigParam postInitParams;
    if(m_operation == 0){
        if(m_keysizeB == 10){
            postInitParams = TConfigParam("PRESENT-80 encryption output config", "", TConfigParam::TType::TDummy, "");
        } else {
            postInitParams = TConfigParam("PRESENT-128 encryption output config", "", TConfigParam::TType::TDummy, "");
        }
    } else {
        if(m_keysizeB == 10){
            postInitParams = TConfigParam("PRESENT-80 decryption output config", "", TConfigParam::TType::TDummy, "");
        } else {
            postInitParams = TConfigParam("PRESENT-128 decryption output config", "", TConfigParam::TType::TDummy, "");
        }
    }

    if(m_operation == 0){ // Encryption
        TConfigParam interValPar = TConfigParam("Intermediate value", "Ciphertext", TConfigParam::TType::TEnum, "Intermediate value to return");
        interValPar.addEnumValue("Plaintext");
        interValPar.addEnumValue("After Nth key addition");
        interValPar.addEnumValue("After Nth SBox-Layer");
        interValPar.addEnumValue("After Nth P-Layer");
        interValPar.addEnumValue("Ciphertext");
        interValPar.addSubParam(TConfigParam("N", "1", TConfigParam::TType::TUShort, "Value N, greater than 0"));
        postInitParams.addSubParam(interValPar);
    } else { // Decryption
        TConfigParam interValPar = TConfigParam("Intermediate value", "Plaintext", TConfigParam::TType::TEnum, "Intermediate value to return");
        interValPar.addEnumValue("Ciphertext");
        interValPar.addEnumValue("After Nth key addition");
        interValPar.addEnumValue("After Nth Inv P-Layer");
        interValPar.addEnumValue("After Nth Inv SBox-Layer");
        interValPar.addEnumValue("Plaintext");
        interValPar.addSubParam(TConfigParam("N", "1", TConfigParam::TType::TUShort, "Value N, greater than 0"));
        postInitParams.addSubParam(interValPar);
    }

    TConfigParam outPar = TConfigParam("Output", "Whole block", TConfigParam::TType::TEnum, "Stream returns either 8 bytes for Whole block, or a byte when restricted to byte or bit");
    outPar.addEnumValue("Whole block");
    outPar.addEnumValue("Mth byte (leftmost byte = 0)");
    outPar.addEnumValue("Mth bit (rightmost bit = 0)");
    outPar.addSubParam(TConfigParam("M", "0", TConfigParam::TType::TUShort, "Non-negative value M lesser than 8 or 64 respectively"));
    postInitParams.addSubParam(outPar);

    m_postInitParams = postInitParams;

    m_analActions.append(new TCipherAction((m_operation==0) ? "Encrypt input data (+ flush streams)" : "Decrypt input data (+ flush streams)", "", [=](){ computeIntermediates(); }));
    m_analActions.append(new TCipherAction("Load cipher key (+ flush streams)", "", [=](){ loadKey(); }));
    m_analActions.append(new TCipherAction("Reset (delete all data)", "", [=](){ reset(); }));


    m_analOutputStreams.append(new TCipherOutputStream((m_operation == 0) ? "Plaintext" : "Ciphertext", "Stream of input data", [=](const uint8_t * buffer, size_t length){ return addData(buffer, length); }));
    m_analOutputStreams.append(new TCipherOutputStream("Cipher key", "Stream of input data", [=](const uint8_t * buffer, size_t length){ return addKeyData(buffer, length); }));

    m_analInputStreams.append(new TCipherInputStream("Intermediate values", "Stream of output data as selected in configuration", [=](uint8_t * buffer, size_t length){ return getIntermediates(buffer, length); }, [=](){ return availableBytes(); }));


    if (ok != nullptr) *ok = true;

}

void TPRESENTEngine::deInit(bool *ok) {

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

TConfigParam TPRESENTEngine::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TPRESENTEngine::setPostInitParams(TConfigParam params) {

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
        } else if (interVal == "After Nth key addition") {
            m_breakpoint = 1;
        } else if (interVal == "After Nth SBox-Layer") {
            m_breakpoint = 2;
        } else if (interVal == "After Nth P-Layer") {
            m_breakpoint = 3;
        } else if (interVal == "Ciphertext") {
            m_breakpoint = 4;
        } else {
            interValPar->setState(TConfigParam::TState::TError, "Invalid intermediate value");
        }
    } else { // Decryption
        if(interVal == "Ciphertext") {
            m_breakpoint = 0;
        } else if (interVal == "After Nth key addition") {
            m_breakpoint = 1;
        } else if (interVal == "After Nth Inv P-Layer") {
            m_breakpoint = 2;
        } else if (interVal == "After Nth Inv SBox-Layer") {
            m_breakpoint = 3;
        } else if (interVal == "Plaintext") {
            m_breakpoint = 4;
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
    } else if(MValUInt > 7 && m_outputRestrict == 1) {
        MValPar->setState(TConfigParam::TState::TError, "M value must be lower than 8");
    } else if(MValUInt > 63 && m_outputRestrict == 2) {
        MValPar->setState(TConfigParam::TState::TError, "M value must be lower than 64");
    } else {
        m_outputRestrictM = MValUInt;
    }

    return m_postInitParams;
}

QList<TAnalAction *> TPRESENTEngine::getActions() const
{
    return m_analActions;
}

QList<TAnalInputStream *> TPRESENTEngine::getInputDataStreams() const
{
    return m_analInputStreams;
}

QList<TAnalOutputStream *> TPRESENTEngine::getOutputDataStreams() const
{
    return m_analOutputStreams;
}

bool TPRESENTEngine::isBusy() const
{
    return false;
}

size_t TPRESENTEngine::addData(const uint8_t * buffer, size_t length){

    m_data.reserve(m_data.size() + length); // preallocate space

    for(int i = 0; i < length; i++){
        m_data.append(buffer[i]);
    }

    return length;

}

size_t TPRESENTEngine::addKeyData(const uint8_t * buffer, size_t length){

    m_keyData.reserve(m_keyData.size() + length); // preallocate space

    for(int i = 0; i < length; i++){
        m_keyData.append(buffer[i]);
    }

    return length;

}

void TPRESENTEngine::reset() {

    m_data.clear();
    m_keyData.clear();
    m_intermediates.clear();
    m_position = 0;

    qInfo("All previously submitted or computed (unread) data have been erased.");

    // TODO clear key

}

size_t TPRESENTEngine::getIntermediates(uint8_t * buffer, size_t length){

    size_t sent = 0;

    for (sent = 0; m_position < m_intermediates.size() && sent < length; sent++, m_position++) {
        buffer[sent] = m_intermediates[m_position];
    }

    return sent;

}

size_t TPRESENTEngine::availableBytes(){
    return m_intermediates.size() - m_position;
}

void TPRESENTEngine::loadKey(){

    if(m_keyData.length() != m_keysizeB){
        qCritical("Key buffer does not contain a valid amount of bytes (10 B for PRESENT-80, 16 B for PRESENT-128). Consider running the Reset action.");
        return;
    }

    for(int i = 0; i < m_keysizeB; i++){
        m_key[i] = m_keyData[i];
    }

    qInfo(QString("The cipher key (%1 bytes) was succesfully set.").arg(m_keysizeB).toLatin1());

    m_keyData.clear();

}

void TPRESENTEngine::computeIntermediates(){

    if(m_data.length() % 8 != 0){
        qCritical("Plaintext/Ciphertext buffer does not contain a valid amount of bytes (not divisible by 8)");
        return;
    }

    // flush stream
    m_intermediates.clear();
    m_position = m_intermediates.length();

    size_t blocksN = m_data.length() / 8;

    qInfo(QString("Unread previously generated data were erased. Now processing %1 bytes of data (%2 cipher blocks).").arg(m_data.length()).arg(blocksN).toLatin1());

    uint8_t PRESENTOut[8];

    m_intermediates.reserve(m_intermediates.size() + m_data.length());

    for(int block = 0; block < blocksN; block++){

        uint8_t * PRESENTIn = &(m_data[block * 8]);

        if(m_operation == 0){

            TPRESENT::EncryptBlock(PRESENTOut, PRESENTIn, m_key, m_keysizeB, m_breakpoint, m_breakpointN);

        } else {

            TPRESENT::DecryptBlock(PRESENTOut, PRESENTIn, m_key, m_keysizeB, m_breakpoint, m_breakpointN);

        }

        if(m_outputRestrict == 0){ // return whole block
            for(int byte = 0; byte < 8; byte++){
                m_intermediates.append(PRESENTOut[byte]);
            }
        } else if(m_outputRestrict == 1) { // Mth byte
            m_intermediates.append(PRESENTOut[m_outputRestrictM]);
        } else { // Mth bit
            uint8_t bitValue = (PRESENTOut[7 - (m_outputRestrictM / 8)] >> (m_outputRestrictM % 8)) & 0x01;
            m_intermediates.append(bitValue);
        }

    }

    m_data.clear();

    qInfo(QString("Generated %1 bytes of data, now available for reading.").arg(m_intermediates.length()).toLatin1());

}
