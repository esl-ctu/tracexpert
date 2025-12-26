// COPYRIGHT HEADER BEGIN
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
// Adam Švehla (initial author)
// David Pokorný
// COPYRIGHT HEADER END

#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include "tprotocol.h"
#include <iostream>
#include <random>
#include <string>
#include <QVariant>
#include <QRandomGenerator>
using namespace Qt;



bool is_big_endian(void)
{
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream stream(stdout);
    bool ok;

/*
    // test command message generation
    TMessage c1("Encipher command", "This command instructs device to encipher supplied payload.", false);

    TMessagePart header("Header", "", TMessagePart::TType::TByte, false);
    header.setByteValue('i', &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(header, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart payload("Payload", "", TMessagePart::TType::TByteArray, true, {}, true, 4);

    c1.addMessagePart(payload, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart ending("Ending", "", TMessagePart::TType::TByte, false);
    ending.setByteValue('\n', &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(ending, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.validateMessage();
    stream << c1.getStateMessage() << endl;

    c1.getMessagePartByName("Payload")->setValue(4567, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    stream << c1.getMessagePartByName("Payload", &ok)->getValue().toHex() << endl;
    stream << (ok ? "Ok" : "Error") << endl;

    c1.validateMessage();
    stream << c1.getStateMessage() << endl;

    stream << c1.getLength() << endl;

    uint8_t buffer[100];
    stream << c1.getData(buffer, 100) << endl;

    for(int i = 0; i < 6; i++) {
        stream << hex << (unsigned char)buffer[i] << " ";
    }
    stream << endl;
*/

/*
    // test static length response message matching
    TProtocol p1("Protocol 1", "empty description");

    TMessage c1("Encipher response", "This response returns the enciphered payload.", true);

    TMessagePart header("Header", "", TMessagePart::TType::TByte, false);
    header.setByteValue('i', &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(header, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart payload("Payload", "", TMessagePart::TType::TByteArray, true, {}, true, 4);

    c1.addMessagePart(payload, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart ending("Ending", "", TMessagePart::TType::TByte, false);
    ending.setByteValue('\n', &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(ending, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.validateMessage();
    stream << c1.getStateMessage() << endl;

    stream << c1.getLength() << endl;

    p1.addMessage(c1, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    uint8_t rbuffer[] = {'i', 0xd7, 0x11, 0, 0, '\n'};

    const TMessage * matchedMessage = p1.tryMatchResponse(rbuffer, 6);

    if(matchedMessage != nullptr) {

        uint8_t buffer[100];

        stream << matchedMessage->getData(buffer, 100) << endl;

        for(int i = 0; i < matchedMessage->getLength(); i++) {
            stream << hex << (unsigned char)buffer[i] << " ";
        }
        stream << endl;

    }

*/

/*
    // test dynamic length response message matching
    TProtocol p1("Protocol 1", "empty description");

    TMessage c1("Encipher response", "This response returns the enciphered payload.", true);

    TMessagePart header("Header", "", TMessagePart::TType::TByte, false);
    header.setByteValue('i', &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(header, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart lengthspec("Length specifier", "", TMessagePart::TType::TByte, true);
    lengthspec.setByteValue(60, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(lengthspec, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart payload("Payload", "", TMessagePart::TType::TByteArray, true, {}, false, 1);

    c1.addMessagePart(payload, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart ending("Ending", "", TMessagePart::TType::TByte, false);
    ending.setByteValue('\n', &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(ending, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.validateMessage();
    stream << c1.getStateMessage() << endl;

    stream << c1.getLength() << endl;

    p1.addMessage(c1, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    uint8_t rbuffer[] = {'i', 3, 0xd7, 0x11, 0, '\n'};

    const TMessage * matchedMessage = p1.tryMatchResponse(rbuffer, 6);

    if(matchedMessage != nullptr) {

        uint8_t buffer[100];

        stream << matchedMessage->getData(buffer, 100) << endl;

        for(int i = 0; i < matchedMessage->getLength(); i++) {
            stream << hex << (unsigned char)buffer[i] << " ";
        }
        stream << endl;

    }
*/

    TProtocol p1("Protocol 1", "empty description");

    {
        TMessage m1("Vstup", "vstup", false);

        TMessagePart header("String na zacatku", "", TMessagePart::TType::TString, false, {}, true, 3);
        header.setValue("abc", &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m1.addMessagePart(header, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart constant("just constant value", "", TMessagePart::TType::TByte, false);
        constant.setByteValue(0x58, &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m1.addMessagePart(constant, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart len1("len in int", "", TMessagePart::TType::TChar, false);
        len1.setValue((signed char)3, &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m1.addMessagePart(len1, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        char databuff[3] = {};
        QByteArray byteArray(databuff, sizeof(databuff));
        TMessagePart data("mydata", "", TMessagePart::TType::TByteArray, false, {}, true, sizeof(databuff));
        data.setValue(byteArray, &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m1.addMessagePart(data, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart intlen("lenghtbased on uint", "", TMessagePart::TType::TUInt, false);
        intlen.setValue(10, &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m1.addMessagePart(intlen, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart data2("mydata2", "", TMessagePart::TType::TString, false, {}, true, 10);
        data2.setValue("String 890", &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m1.addMessagePart(data2, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        m1.validateMessage();
        stream << "state m1: " << (int)m1.getState() << endl;

        p1.addMessage(m1, &ok);
        stream << (ok ? "Ok" : "Error") << endl;
    }



    {
        TMessage m2("Vystup", "", true);

        TMessagePart header("String na zacatku", "", TMessagePart::TType::TString, false, {}, true, 3);
        header.setValue("abc", &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m2.addMessagePart(header, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart constant("just constant value", "", TMessagePart::TType::TByte, false);
        constant.setByteValue(0x58, &ok);
        stream << (ok ? "Ok" : "Error") << endl;
        m2.addMessagePart(constant, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart len1("len in int", "", TMessagePart::TType::TChar, true, {}, true, 0, false);
        //len1.setValue((signed char)3, &ok);
        //stream << (ok ? "Ok" : "Error") << endl;
        m2.addMessagePart(len1, &ok);
        stream << (ok ? "Ok" : "Error") << endl;


        TMessagePart data("mydata", "", TMessagePart::TType::TByteArray, true, {}, false, 2);
        //data.setValue({}, &ok);
        //stream << (ok ? "Ok" : "Error") << endl;
        m2.addMessagePart(data, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart intlen("lenghtbased on uint", "", TMessagePart::TType::TUInt, true, {}, true, 0, false);
        //intlen.setValue(0, &ok);
        //stream << (ok ? "Ok" : "Error") << endl;
        m2.addMessagePart(intlen, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart data2("mydata2", "", TMessagePart::TType::TString, true, {}, false, 4);
        //data2.setValue("String 890", &ok);
        //stream << (ok ? "Ok" : "Error") << endl;
        m2.addMessagePart(data2, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        m2.validateMessage();
        stream << (int) m2.getState() << endl;
        stream << "delka m2: " << m2.getLength() << endl;

        p1.addMessage(m2, &ok);
        stream << (ok ? "Ok" : "Error") << endl;
    }

    {
        auto m1 = p1.getMessageByName("Vstup");
        if(m1 == nullptr) stream << "vstup nenalezen" << endl;
        else{
            qsizetype m1len = m1->getLength();
            stream << "state m1: " << m1->getStateMessage() << endl;
            stream << "delka m1: " << m1len << endl;

            uint8_t buffer[100];
            qsizetype size = m1->getData(buffer, 100);
            stream << "Message vstup:" << endl;
            for(int i = 0; i < size; i++) {
                stream << hex << (unsigned char)buffer[i] << " ";
            }
            stream << endl;




            TMessage * matchedMessage = p1.tryMatchResponse(buffer, size);

            if(matchedMessage != nullptr) {

                uint8_t buffer[100];

                stream << matchedMessage->getData(buffer, 100) << endl;

                for(int i = 0; i < matchedMessage->getLength(); i++) {
                    stream << hex << (unsigned char)buffer[i] << " ";
                }
                stream << endl;


                {
                    //získání dat ze zprávy: "mydata2"
                    QString MessagePartName = "mydata2";
                    TMessagePart * data = matchedMessage->getMessagePartByName(MessagePartName);
                    stream << "data from:" << MessagePartName << endl;
                    data->getData(buffer, 100);
                    auto len = matchedMessage->getMessagePartLengthByName(MessagePartName);
                    for(int i = 0; i < len ; i++) {
                        stream << hex << buffer[i] << " ";
                    }
                    stream << endl;
                }

                {
                    //získání dat ze zprávy: "len in int"
                    QString MessagePartName = "len in int";
                    TMessagePart * data = matchedMessage->getMessagePartByName(MessagePartName);
                    stream << "data from:" << MessagePartName << endl;
                    data->getData(buffer, 100);
                    auto len = matchedMessage->getMessagePartLengthByName(MessagePartName);
                    for(int i = 0; i < len ; i++) {
                        stream << hex << buffer[i] << " ";
                    }
                    stream << endl;
                }


            }else stream << "not found match" << endl;
        }
    }

    //kontrola endianity
    {

        stream << "my pc is " << (is_big_endian() ? "big-endian" : "little-endian") << endl; // v tomto testu by na tom nemelo zalezet

        TMessage m("Response endian", "", true);

        TMessagePart intlen("lenghtbased on uint", "", TMessagePart::TType::TUInt, true, {}, true, 0, true); //vstup je little-endian

        m.addMessagePart(intlen, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        TMessagePart data2("mydata2", "", TMessagePart::TType::TString, true, {}, false, 0);
        m.addMessagePart(data2, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        m.validateMessage();
        stream << (int) m.getState() << endl;
        stream << m.getLength() << endl;

        p1.addMessage(m, &ok);
        stream << (ok ? "Ok" : "Error") << endl;

        uint8_t buffin[] = {0x02, 0x00, 0x00, 0x00, 0x10, 0x11}; // 2 v little-endian a pak 2 bajty dat
        for(uint8_t* p = buffin; p<buffin+sizeof(buffin); p++){
            stream << hex << *p << " ";
        }
        stream << endl;

        const TMessage * matchedMessage = p1.tryMatchResponse(buffin, sizeof(buffin));
        if(matchedMessage != nullptr) {

            uint8_t buffer[100];

            stream << matchedMessage->getData(buffer, 100) << endl;

            for(int i = 0; i < matchedMessage->getLength(); i++) {
                stream << hex << (unsigned char)buffer[i] << " ";
            }
            stream << endl;

        }else stream << "not found match" << endl;
    }





	return 0;
}
