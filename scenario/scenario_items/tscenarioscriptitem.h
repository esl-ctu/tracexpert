#ifndef TSCENARIOSCRIPTITEM_H
#define TSCENARIOSCRIPTITEM_H

#include "../tscenarioitem.h"

#include <QProcess>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>

/*!
 * \brief ...
 *
 */
class TScenarioScriptItem : public TScenarioItem {

public:
    TItemClass itemClass() const override {
        return TItemClass::TScenarioScriptItem;
    }

    TScenarioScriptItem() : TScenarioItem(tr("Python script"), tr("This block runs a Python script.")) {
        addFlowInputPort("flowIn");
        addFlowOutputPort("flowOut");

        addDataInputPort("dataIn0", "", tr("Data will be passed to the script through this port."), "[any]");
        addDataInputPort("dataIn1", "", tr("Data will be passed to the script through this port."), "[any]");
        addDataOutputPort("dataOut0", "", tr("Data from the the script will be returned through this port."), "[any]");

        m_inputPortCount = 2;
        m_outputPortCount = 1;

        m_params = TConfigParam(m_name + " configuration", "", TConfigParam::TType::TDummy, "");
        m_params.addSubParam(TConfigParam("Block name", "Python script", TConfigParam::TType::TString, tr("Display name of the block."), false));
        m_params.addSubParam(TConfigParam("Python path", "", TConfigParam::TType::TFileName, tr("The python interpreter (executable) path."), false));
        m_params.addSubParam(TConfigParam("Input count", "2", TConfigParam::TType::TUInt, "Number of data inputs of the block between 0 and 10.", false));
        m_params.addSubParam(TConfigParam("Output count", "1", TConfigParam::TType::TUInt, "Number of data outputs of the block between 0 and 10.", false));
        m_params.addSubParam(TConfigParam("Script", R"("""
Instructions:
-------------
- There are convenience functions for input conversion:
    tstring(i): UTF-8 string from input_raw[i]
    tint(i):    32-bit int from input_raw[i]
    treal(i):   double from input_raw[i]
    tbool(i):   bool from first byte of input_raw[i]
    and tuint, tshort, tushort, tlonglong, tulonglong...
- You can also access input_raw directly.
- Return outputs as a list.
"""

def process_data():
    sum = tint(0) + tint(1)
    return [sum]
)", TConfigParam::TType::TCode, tr("The python code to execute."), false));

    }

    ~TScenarioScriptItem() {
        cleanupProcess();
    }

    bool supportsDirectExecution() const override {
        return false;
    }

    TScenarioItem * copy() const override {
        return new TScenarioScriptItem(*this);
    }

    /*const QString getIconResourcePath() const override {
        return ":/icons/variable.png";
    }*/

    bool prepare() override {
        if(getState() == TState::TError) {
            return false;
        }

        return true;
    }

    bool validateParamsStructure(TConfigParam params) override {
        bool iok = false;

        params.getSubParamByName("Block name", &iok);
        if(!iok) return false;

        params.getSubParamByName("Python path", &iok);
        if(!iok) return false;

        params.getSubParamByName("Input count", &iok);
        if(!iok) return false;

        params.getSubParamByName("Output count", &iok);
        if(!iok) return false;

        params.getSubParamByName("Script", &iok);
        if(!iok) return false;

        return true;
    }

    TConfigParam setParams(TConfigParam params) override {
        if(!validateParamsStructure(params)) {
            params.setState(TConfigParam::TState::TError, tr("Wrong structure of the pre-init params."));
            return params;
        }

        m_params = params;
        m_params.resetState(true);

        if(m_title != params.getSubParamByName("Block name")->getValue()) {
            m_title = params.getSubParamByName("Block name")->getValue();
            emit appearanceChanged();
        }

        // generate input ports
        quint32 inputCount = m_params.getSubParamByName("Input count")->getValue().toUInt();
        if(inputCount > 10) {
            m_params.getSubParamByName("Input count")->setState(TConfigParam::TState::TError, "Value has to be between 0 and 10.");
            return m_params;
        }

        if(inputCount != m_inputPortCount) {
            for(quint32 i = 0; i < std::max(inputCount, m_inputPortCount); i++) {
                if(i >= inputCount) {
                    // remove this port
                    removePort(QString("dataIn%1").arg(i));
                }
                else {
                    // create a new port
                    addDataInputPort(QString("dataIn%1").arg(i), "", tr("Data will be passed to the script through this port."), "[any]");
                }
            }

            m_inputPortCount = inputCount;
        }

        // generate output ports
        quint32 outputCount = m_params.getSubParamByName("Output count")->getValue().toUInt();
        if(outputCount > 10) {
            m_params.getSubParamByName("Output count")->setState(TConfigParam::TState::TError, "Value has to be between 0 and 10.");
            return m_params;
        }

        if(outputCount != m_outputPortCount) {
            for(quint32 i = 0; i < std::max(outputCount, m_outputPortCount); i++) {
                if(i >= outputCount) {
                    // remove this port
                    removePort(QString("dataOut%1").arg(i));
                }
                else {
                    // create a new port
                    addDataOutputPort(QString("dataOut%1").arg(i), "", tr("Data from the the script will be returned through this port."), "[any]");
                }
            }

            m_outputPortCount = outputCount;
        }


        if(m_params.getState(true) == TConfigParam::TState::TError) {
            setState(TState::TError, tr("Block configuration contains errors!"));
        }
        else {
            resetState();
        }

        return m_params;
    }

    void processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
       /* if (!m_pythonProcess->waitForFinished()) {
            setState(TState::TRuntimeError, "Python process did not finish!");
            log("Python process did not finish!", TLogLevel::TError);
            emit executionFinished();
            return;
        } */

        if (!m_pythonProcess) {
           setState(TState::TRuntimeError, "An error has occured during script execution.");
           log("An error has occured during script execution.", TLogLevel::TError);
           emit executionFinished();
           return;
        }

        QByteArray errorOutput = m_pythonProcess->readAllStandardError();
        if (!errorOutput.isEmpty()) {
            log(QString("Python process finished with errors in stderr:\n%1").arg(errorOutput), TLogLevel::TWarning);
        }

        // Read and decode response
        QByteArray response = m_pythonProcess->readAll();

        if(exitCode > 0) {
            setState(TState::TRuntimeError, response);
            log(QString("Python process finished with exit code %1").arg(exitCode), TLogLevel::TError);

            cleanupProcess();
            emit executionFinished();
            return;
        }

        QHash<TScenarioItemPort *, QByteArray> decodedData;
        if (!decodeData(response, decodedData)) {
            setState(TState::TRuntimeError, "Failed to decode output data!");
            log("Failed to decode output data!", TLogLevel::TError);

            cleanupProcess();
            emit executionFinished();
            return;
        }

        cleanupProcess();
        emit executionFinished(decodedData);
    }

    void stopExecution() override {
        if(m_pythonProcess) {
            m_pythonProcess->terminate();
        }

        emit executionFinished();
    }

    void terminateExecution() override {
        if(m_pythonProcess) {
            m_pythonProcess->kill();
        }
    }

    void cleanupProcess() {
        if(m_pythonProcess) {
            m_pythonProcess->deleteLater();
            m_pythonProcess = nullptr;
        }
    }

    void executeIndirect(const QHash<TScenarioItemPort *, QByteArray> & inputData) override {
        cleanupProcess();

        m_pythonProcess = new QProcess();
        connect(m_pythonProcess, &QProcess::finished, this, &TScenarioScriptItem::processFinished);

        if(m_params.getSubParamByName("Python path")->getValue().isEmpty()) {
           m_pythonProcess->setProgram("python");
        }
        else {
            m_pythonProcess->setProgram(m_params.getSubParamByName("Python path")->getValue());
        }

        QString sandboxScript = R"(
import sys
import struct

def read_exactly(n):
    """Reads exactly n bytes from stdin, handling partial reads."""
    data = bytearray()
    while len(data) < n:
        chunk = sys.stdin.buffer.read(n - len(data))
        if not chunk:
            sys.stderr.write("Error: Unexpected end of input\n")
            sys.exit(1)
        data.extend(chunk)
    return bytes(data)

try:
    # === Step 1: Read and execute the user-provided script ===
    # Read 4-byte little-endian integer indicating the length of the user script.
    script_length = struct.unpack("<I", read_exactly(4))[0]
    user_code = read_exactly(script_length).decode('utf-8')

    # Execute the user code in the global namespace.
    exec(user_code, globals())

    # === Step 2: Read raw input data ===
    # Protocol: 1 byte for the number of input elements.
    num_elements = struct.unpack("<B", read_exactly(1))[0]
    input_raw = []
    for i in range(num_elements):
        length = struct.unpack("<I", read_exactly(4))[0]
        raw_data = read_exactly(length)
        input_raw.append(raw_data)

    # === Step 3: Define convenience functions (always little-endian, UTF-8) ===
    def tstring(idx):
        """Converts raw bytes at input_raw[idx] to a UTF-8 string."""
        return input_raw[idx].decode('utf-8')

    def tint(idx):
        """Converts raw bytes at input_raw[idx] to a 32-bit signed integer."""
        return int.from_bytes(input_raw[idx], byteorder='little', signed=True)

    def tuint(idx):
        """Converts raw bytes at input_raw[idx] to a 32-bit unsigned integer."""
        return int.from_bytes(input_raw[idx], byteorder='little', signed=False)

    def tshort(idx):
        """Converts raw bytes at input_raw[idx] to a 16-bit signed integer (expects 2 bytes)."""
        if len(input_raw[idx]) != 2:
            raise ValueError("Expected 2 bytes for tshort")
        return int.from_bytes(input_raw[idx], byteorder='little', signed=True)

    def tushort(idx):
        """Converts raw bytes at input_raw[idx] to a 16-bit unsigned integer (expects 2 bytes)."""
        if len(input_raw[idx]) != 2:
            raise ValueError("Expected 2 bytes for tushort")
        return int.from_bytes(input_raw[idx], byteorder='little', signed=False)

    def tlonglong(idx):
        """Converts raw bytes at input_raw[idx] to a 64-bit signed integer (expects 8 bytes)."""
        if len(input_raw[idx]) != 8:
            raise ValueError("Expected 8 bytes for tlonglong")
        return int.from_bytes(input_raw[idx], byteorder='little', signed=True)

    def tulonglong(idx):
        """Converts raw bytes at input_raw[idx] to a 64-bit unsigned integer (expects 8 bytes)."""
        if len(input_raw[idx]) != 8:
            raise ValueError("Expected 8 bytes for tulonglong")
        return int.from_bytes(input_raw[idx], byteorder='little', signed=False)

    def treal(idx):
        """Converts raw bytes at input_raw[idx] to a double-precision float (expects 8 bytes)."""
        if len(input_raw[idx]) != 8:
            raise ValueError("Expected 8 bytes for treal")
        return struct.unpack("<d", input_raw[idx])[0]

    def tbool(idx):
        """Converts the first byte at input_raw[idx] to a boolean (0 → False, nonzero → True)."""
        if len(input_raw[idx]) < 1:
            raise ValueError("Expected at least 1 byte for tbool")
        return input_raw[idx][0] != 0

    # Make the convenience functions available in the global namespace.
    globals()['tstring'] = tstring
    globals()['tint'] = tint
    globals()['tuint'] = tuint
    globals()['tshort'] = tshort
    globals()['tushort'] = tushort
    globals()['tlonglong'] = tlonglong
    globals()['tulonglong'] = tulonglong
    globals()['treal'] = treal
    globals()['tbool'] = tbool

    # === Step 4: Call the user-defined process_data() function ===
    if "process_data" not in globals():
        sys.stderr.write("Error: process_data function not defined by user.\n")
        sys.exit(1)

    # The user-defined function is expected to return a list of outputs.
    output_data = process_data()

    # === Step 5: Convert outputs to a binary payload ===
    # Protocol: 1 byte for the number of outputs, then for each output a 4-byte little-endian length followed by the raw data.
    output_payload = bytearray()
    output_payload.append(len(output_data))  # Assumes number of items < 256

    for item in output_data:
        if isinstance(item, bytes):
            encoded_value = item
        elif isinstance(item, str):
            encoded_value = item.encode('utf-8')
        elif isinstance(item, int):
            encoded_value = struct.pack("<i", item)
        elif isinstance(item, float):
            encoded_value = struct.pack("<d", item)
        elif isinstance(item, bool):
            encoded_value = struct.pack("<?", item)
        else:
            sys.stderr.write("Error: Unsupported return type\n")
            sys.exit(1)
        output_payload.extend(struct.pack("<I", len(encoded_value)))
        output_payload.extend(encoded_value)

    sys.stdout.buffer.write(output_payload)

except Exception as e:
    sys.stderr.write("Error: " + str(e) + "\n")
    sys.exit(1)
)";

        m_pythonProcess->setArguments(QStringList() << "-c" << sandboxScript);
        m_pythonProcess->setProcessChannelMode(QProcess::SeparateChannels);
        m_pythonProcess->start();

        if (!m_pythonProcess->waitForStarted()) {
            setState(TState::TRuntimeError, "Failed to start Python process!");
            log("Failed to start Python process!", TLogLevel::TError);
            cleanupProcess();
            emit executionFinished();
            return;
        }

        // Encode user function with length prefix
        QByteArray userCodeBytes = m_params.getSubParamByName("Script")->getValue().toUtf8();
        QByteArray lengthPrefix;
        uint32_t scriptLength = userCodeBytes.size();
        lengthPrefix.append(reinterpret_cast<const char *>(&scriptLength), sizeof(scriptLength));

        m_pythonProcess->write(lengthPrefix);
        m_pythonProcess->waitForBytesWritten();
        m_pythonProcess->write(userCodeBytes);
        m_pythonProcess->waitForBytesWritten();

        // Encode and send input data
        QByteArray encodedData;
        if (!encodeData(inputData, encodedData)) {
            setState(TState::TRuntimeError, "Failed to encode input data!");
            log("Failed to encode input data!", TLogLevel::TError);
            cleanupProcess();
            emit executionFinished();
            return;
        }

        m_pythonProcess->write(encodedData);
        m_pythonProcess->waitForBytesWritten();
        m_pythonProcess->closeWriteChannel();
    }

    // Encode data from QHash into proprietary binary format
    bool encodeData(const QHash<TScenarioItemPort *, QByteArray> &inputData, QByteArray &encodedData) {
        int inputCount = m_params.getSubParamByName("Input count")->getValue().toUInt();
        encodedData.append(static_cast<uint8_t>(inputCount));  // Number of input segments

        // For each input, append the 4-byte length immediately followed by its data.
        for (int i = 0; i < inputCount; i++) {
            TScenarioItemPort *port = getItemPortByName(QString("dataIn%1").arg(i));
            if (!port) {
                log(QString("Missing input port: dataIn%1").arg(i), TLogLevel::TError);
                return false;
            }

            QByteArray data;
            if (inputData.contains(port)) {
                data = inputData[port];
            } else {
                log(QString("Missing input port data: dataIn%1").arg(i), TLogLevel::TWarning);
            }

            uint32_t length = data.size();
            encodedData.append(reinterpret_cast<const char *>(&length), sizeof(length));
            encodedData.append(data);
        }

        return true;
    }

    // Decode binary response into QHash using output port names
    bool decodeData(const QByteArray &response, QHash<TScenarioItemPort *, QByteArray> &decodedData) {
        int outputCount = m_params.getSubParamByName("Output count")->getValue().toUInt();

        if (response.isEmpty()) {
            log("No output data from Python!", TLogLevel::TError);
            return false;
        }

        qsizetype index = 0;
        int numSegments = static_cast<unsigned char>(response[index++]);

        if (numSegments != outputCount) {
            log(QString("Unexpected number of outputs from python! (got %1, expected %2)").arg(numSegments).arg(outputCount), TLogLevel::TError);
            return false;
        }

        // For each output, read a 4-byte length and then the data immediately.
        for (int i = 0; i < numSegments; i++) {
            if (index + sizeof(uint32_t) > response.size()) {
                log(QString("Unexpected end of response when reading length for segment %1").arg(i), TLogLevel::TError);
                return false;
            }
            uint32_t length;
            memcpy(&length, response.constData() + index, sizeof(length));
            index += sizeof(length);

            if (index + length > response.size()) {
                log(QString("Unexpected end of response when reading data for segment %1").arg(i), TLogLevel::TError);
                return false;
            }
            QByteArray segment = response.mid(index, length);
            index += length;

            TScenarioItemPort *port = getItemPortByName(QString("dataOut%1").arg(i));
            if (!port) {
                log(QString("Missing output port: dataOut%1").arg(i), TLogLevel::TError);
                return false;
            }
            decodedData.insert(port, segment);
        }

        return true;
    }

private:
    QProcess * m_pythonProcess = nullptr;

    quint32 m_inputPortCount;
    quint32 m_outputPortCount;

};

#endif // TSCENARIOSCRIPTITEM_H
