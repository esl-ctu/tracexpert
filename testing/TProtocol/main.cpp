#include <QCoreApplication>
#include <QPluginLoader>
#include <QDir>
#include "tprotocol.h"
using namespace Qt;

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

    // test dynamic length response message matching
    TProtocol p1("Protocol 1", "empty description");

    TMessage c1("Encipher response", "This response returns the enciphered payload.", true);

    TMessagePart header("Header", "", TMessagePart::TType::TByte, false);
    header.setByteValue('i', &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    c1.addMessagePart(header, &ok);
    stream << (ok ? "Ok" : "Error") << endl;

    TMessagePart lengthspec("Length specifier", "", TMessagePart::TType::TByte, true);
    lengthspec.setByteValue(5, &ok);
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
	return 0;
}
