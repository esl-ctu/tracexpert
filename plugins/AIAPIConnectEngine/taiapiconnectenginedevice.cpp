#include "taiapiconnectenginedevice.h"

//todo
//timeout
//send get/post request

TAIAPIConnectEngineDevice::TAIAPIConnectEngineDevice(QString name, QString info) {
    m_name = name;
    m_info = info;
    m_initialized = false;

    m_preInitParams = TConfigParam("AIAPIConnectEngineDevice pre-init config", "", TConfigParam::TType::TDummy, "");
    m_postInitParams = TConfigParam("AIAPIConnectEngineDevice post-init config", "", TConfigParam::TType::TDummy, "");
    TConfigParam address = TConfigParam("Address", "127.0.0.1", TConfigParam::TType::TString, "Address to connect to (not a hostname)");
    TConfigParam port = TConfigParam("Port", "8000", TConfigParam::TType::TInt, "Port to connect to");
    TConfigParam inputSize = TConfigParam("Input size", "30000", TConfigParam::TType::TInt, "How big should ech individual input to the model be. Reserved for future use, not checked.", true);
    //TConfigParam separator = TConfigParam("Separator", "\n", TConfigParam::TType::TString, "Separator for individual values in the input. Defaults to newline.");
    auto type = TConfigParam("Data type", QString("int32_t"), TConfigParam::TType::TEnum, "If type is Text, datapoints must be separated by a newline");
    type.addEnumValue("int16_t");
    type.addEnumValue("int32_t");
    type.addEnumValue("int64_t");
    type.addEnumValue("unt16_t");
    type.addEnumValue("unt32_t");
    type.addEnumValue("unt64_t");
    type.addEnumValue("Double");
    type.addEnumValue("Text");

    m_preInitParams.addSubParam(address);
    m_preInitParams.addSubParam(port);
    m_preInitParams.addSubParam(inputSize);
    //m_preInitParams.addSubParam(separator);
    m_preInitParams.addSubParam(type);

    running = false;
    dataReady = false;
}

TAIAPIConnectEngineDevice::~TAIAPIConnectEngineDevice() {}

QString TAIAPIConnectEngineDevice::getName() const {
    return m_name;
}

QString TAIAPIConnectEngineDevice::getInfo() const {
    return m_info;
}

TConfigParam TAIAPIConnectEngineDevice::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TAIAPIConnectEngineDevice::setPreInitParams(TConfigParam params) {
    if(m_initialized){
        params.setState(TConfigParam::TState::TError, "Cannot change pre-init parameters on an initialized device.");
        return params;
    }

    bool ok = true;
    int port = params.getSubParamByName("Port")->getValue().toInt();
    QString addr = params.getSubParamByName("Address")->getValue();
    //QString sep = params.getSubParamByName("Separator")->getValue();

    if (port < 80 || port > 65000) {
        ok = false;
        params.getSubParamByName("Port")->setState(TConfigParam::TState::TError, "Port number invalid.");
    }

    QHostAddress address(addr);
    if (QAbstractSocket::IPv4Protocol == address.protocol()) {
        //all good
    } else if (QAbstractSocket::IPv6Protocol == address.protocol()){
        params.getSubParamByName("Address")->setState(TConfigParam::TState::TError, "IPv6 is not supported");
        ok = false;
    } else {
        params.getSubParamByName("Address")->setState(TConfigParam::TState::TError, "Address invalid");
        ok = false;
    }

    /*if (sep.length() != 1) {
        ok = false;
        params.getSubParamByName("Separator")->setState(TConfigParam::TState::TError, "Separator invalid");
    }*/

    if (ok) {
        m_preInitParams = params;
        return m_preInitParams;
    } else {
        return params;
    }
}

void TAIAPIConnectEngineDevice::init(bool *ok /*= nullptr*/) {
    m_analActions.append(new TAIAPIConnectEngineDeviceAction("Analyze", "Runs the specified model on provided input data.", [=](){ analyzeData(); }));
    m_analInputStreams.append(new TAIAPIConnectEngineDeviceInputStream("Result", "Stream of results of last used action", [=](uint8_t * buffer, size_t length){ return getData(buffer, length); }));
    m_analOutputStreams.append(new TAIAPIConnectEngineDeviceOutputStream("Input", "Stream of traces", [=](const uint8_t * buffer, size_t length){ return fillData(buffer, length); }));

    if (ok != nullptr) *ok = true;
    int port = m_preInitParams.getSubParamByName("Port")->getValue().toInt();
    QString addr = m_preInitParams.getSubParamByName("Address")->getValue();
    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    QUrl url;
    url.setHost(addr);
    url.setPort(port);
    url.setScheme("http");
    QNetworkRequest request(url);

    QNetworkReply* reply = mgr->get(request);

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QAbstractSocket::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QAbstractSocket::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(10000);   // 10 secs. timeout
    loop.exec();

    int res = -1;
    if(timer.isActive()) {
        timer.stop();
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        res = statusCode.toInt();
    }
    else {
        QAbstractSocket::disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
        if (ok != nullptr) *ok = false;
        qDebug("Is the python server running?");
    }

    reply->deleteLater();
    delete mgr;

    if (res != 200){
        qDebug("Is the python server running? Invalid http response received.");
        qDebug("%s", QString::number(res).toLocal8Bit().constData());
        if (ok != nullptr) *ok = false;
    }


    m_initialized = true;

    getServerMode();
}

void TAIAPIConnectEngineDevice::deInit(bool *ok /*= nullptr*/) {
    if (ok != nullptr) *ok = true;

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

    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    if (m_data) {
        if (type == "int16_t") delete (int16_t *) m_data;
        else if (type == "int32_t") delete (int32_t *) m_data;
        else if (type == "int64_t") delete (int64_t *) m_data;
        else if (type == "uint16_t") delete (uint16_t *) m_data;
        else if (type == "uint32_t") delete (uint32_t *) m_data;
        else if (type == "uint64_t") delete (uint64_t *) m_data;
        else if (type == "Double") delete (double *) m_data;
        else if (type == "Text") delete (char *) m_data;
        m_data = nullptr;
    }

    m_initialized = false;
}

TConfigParam TAIAPIConnectEngineDevice::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TAIAPIConnectEngineDevice::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams; //Careful! This assumes no postinitparams!
}

QList<TAnalAction *> TAIAPIConnectEngineDevice::getActions() const {
    return m_analActions;
}

QList<TAnalInputStream *> TAIAPIConnectEngineDevice::getInputDataStreams() const {
    return m_analInputStreams;
}

QList<TAnalOutputStream *> TAIAPIConnectEngineDevice::getOutputDataStreams() const {
    return m_analOutputStreams;
}

bool TAIAPIConnectEngineDevice::isBusy() const {
    return running;
}

int getTypeSize(QString type) {
    if (type == "int16_t") return sizeof(int16_t);
    if (type == "int32_t") return  sizeof(int32_t);
    if (type == "int64_t") return  sizeof(int64_t);
    if (type == "uint16_t") return  sizeof(uint16_t);
    if (type == "uint32_t") return  sizeof(uint32_t);
    if (type == "uint64_t") return  sizeof(uint64_t);
    if (type == "Double") return  sizeof(double);
    if (type == "Text") return  1;
    return 0;
}

bool TAIAPIConnectEngineDevice::analyzeData() {
    if (running) {
        return false;
    }
    running = true;
    dataReady = false;


    bool ok = true;
    int port = m_preInitParams.getSubParamByName("Port")->getValue().toInt();
    QString addr = m_preInitParams.getSubParamByName("Address")->getValue();
    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    int inputItemSize = m_preInitParams.getSubParamByName("Input size")->getValue().toInt();
    uint8_t typeSize = getTypeSize(type);

    if (m_length % inputItemSize == 0 && type != "Text") {
        qDebug("Uneven number of samples received (1)");
        running = false;
        return false;
    }

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        running = false;
        return false;
    }

    QJsonObject param;
    QJsonArray jsonArray;

    if (type == "Text") {
        char * data = (char *) m_data;
        QString dataS(data);
        QStringList lines_in = dataS.split('\n', Qt::SkipEmptyParts);
        QStringList lines;

        QRegularExpression regex("([+-]?[0-9]*[\\.][0-9]+|[+-]?[0-9]+)");
        QRegularExpressionMatch match;
        for (QString &line : lines_in) {
            line = line.trimmed();  // Remove leading and trailing whitespace
            match = regex.match(line); //preserve numbers only
            if (match.hasMatch()) {
                lines << match.captured(0);
            }
        }

        if (lines.length() % inputItemSize != 0) {
            qDebug() << lines.length();
            qDebug("Uneven number of samples received (2)");
            running = false;
            return false;
        }

        bool ook = true;
        for (const QString &str : lines) {
            bool oook;
            double number = str.toDouble(&oook);
            if (oook) jsonArray.append(number);
            else ook = false;
        }

        if (!ook) {
            qDebug("Input data invalid");
            running = false;
            return false;
        }
    } else {
        for (int i = 0; i < m_length; ++i)  {
            if (type == "int16_t") jsonArray.append(((int16_t *) m_data)[i]);
            if (type == "int32_t") jsonArray.append(((int32_t *) m_data)[i]);
            if (type == "int64_t") jsonArray.append(((int64_t *) m_data)[i]);
            if (type == "uint16_t") jsonArray.append(((uint16_t *) m_data)[i]);
            if (type == "uint32_t") {
                int64_t tmp = ((uint32_t *) m_data)[i];
                jsonArray.append(tmp);
            }
            if (type == "uint64_t") {
                if (((uint64_t *) m_data)[i] > INT64_MAX) {
                    double tmp = ((uint64_t *) m_data)[i];
                    jsonArray.append(tmp);
                } else {
                    int64_t tmp = ((uint64_t *) m_data)[i];
                    jsonArray.append(tmp);
                }

            }
            if (type == "Double") jsonArray.append(((double *) m_data)[i]);
        }

        if (jsonArray.size() % inputItemSize != 0) {
            qDebug("Uneven number of samples received (3)");
            running = false;
            return false;
        }
    }

    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    QUrl url;
    url.setHost(addr);
    url.setPort(port);
    url.setScheme("http");
    url.setPath("/predict");
    QNetworkRequest request(url);

    param["data"] = jsonArray;
    QJsonDocument doc(param);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = mgr->post(request, data);

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QAbstractSocket::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QAbstractSocket::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(10000);   // 10 secs. timeout
    loop.exec();

    QByteArray replyBuffer;
    if(timer.isActive()) {
        timer.stop();
        if(reply->error() == QNetworkReply::NoError){
            replyBuffer = reply->readAll();
        }
        else {
            QString error = reply->errorString();
            qDebug()<< "reply->errorString() " << error;
            ok = false;
        }
    }
    else {
        QAbstractSocket::disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
        ok = false;
    }

    reply->deleteLater();
    delete mgr;

    if (!ok){
        qDebug("Model reported invalid data");
        running = false;
        return false;
    }

    QJsonDocument jsonResponse = QJsonDocument::fromJson(replyBuffer);

    if (!jsonResponse.isObject()) {
        qDebug() << "Invalid JSON response";
        running = false;
        return false;
    }

    QJsonObject jsonObject = jsonResponse.object();
    if (!jsonObject.contains("prediction")) {
        qDebug() << "Prediction field not found in JSON response";
        running = false;
        return false;
    }

    QJsonArray predictionArray = jsonObject["prediction"].toArray();

    if (type == "Text") {
        QString outS;
        bool start = true;
        for (const QJsonValue &value : predictionArray) {
            if (!start) outS += '\n';
            outS += (QString::number(value.toInt()));
            start = false;
        }

        QByteArray byteArray = outS.toUtf8();
        const char* cStr = byteArray.constData();
        size_t max = byteArray.size();
        if (m_length < max) max = m_length;
        std::memcpy(m_data, cStr, max);
        m_length = max;
    } else {
        size_t i = 0;
        for (const QJsonValue &value : predictionArray) {
            if (i == m_length) break;
            if (type == "int16_t") ((int16_t *) m_data)[i] = value.toInt();
            if (type == "int32_t") ((int32_t *) m_data)[i] = value.toInt();
            if (type == "int64_t") ((int64_t *) m_data)[i] = value.toInt();
            if (type == "uint16_t") ((uint16_t *) m_data)[i] = value.toInt();
            if (type == "uint32_t") ((uint32_t *) m_data)[i] = value.toInt();
            if (type == "uint64_t") ((uint64_t *) m_data)[i] = value.toInt();
            if (type == "Double") ((double *) m_data)[i] = value.toDouble();
            i++;
        }
        m_length = i;

    }

    dataReady = true;
    running = false;

    return true;
}

size_t TAIAPIConnectEngineDevice::getData(uint8_t * buffer, size_t length) {
    if (running) {
        return 0;
    }
    running = true;

    if (!dataReady) {
        running = false;
        return 0;
    }

    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    uint8_t typeSize = getTypeSize(type);

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        running = false;
        return 0;
    }

    if (!m_data) {
        running = false;
        return 0;
    }

    size_t max = length;
    if (length > typeSize*m_length) max = typeSize*m_length;

    memcpy(buffer, m_data, max);

    running = false;

    return max;
}

size_t TAIAPIConnectEngineDevice::fillData(const uint8_t * buffer, size_t length) {
    if (running) {
        return 0;
    }
    running = true;
    dataReady = false;

    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    uint8_t typeSize = getTypeSize(type);

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        running = false;
        return 0;
    }

    if (m_data) {
        if (type == "int16_t") delete (int16_t *) m_data;
        else if (type == "int32_t") delete (int32_t *) m_data;
        else if (type == "int64_t") delete (int64_t *) m_data;
        else if (type == "uint16_t") delete (uint16_t *) m_data;
        else if (type == "uint32_t") delete (uint32_t *) m_data;
        else if (type == "uint64_t") delete (uint64_t *) m_data;
        else if (type == "Double") delete (double *) m_data;
        else if (type == "Text") delete (char *) m_data;
        m_data = nullptr;
    }

    m_length = length / typeSize;

    if (type == "int16_t") m_data = (void *) new int16_t[m_length];
    else if (type == "int32_t") m_data = (void *) new int32_t[m_length];
    else if (type == "int64_t") m_data = (void *) new int64_t[m_length];
    else if (type == "uint16_t") m_data = (void *) new uint16_t[m_length];
    else if (type == "uint32_t") m_data = (void *) new uint32_t[m_length];
    else if (type == "uint64_t") m_data = (void *) new uint64_t[m_length];
    else if (type == "Double") m_data = (void *) new double[m_length];
    else if (type == "Text") m_data = (void *) new char[m_length + 1];
    else {
        qDebug("Invalid type of input (2)");
        running = false;
        return 0;
    }

    memcpy(m_data, buffer, length);
    if (type == "Text") ((char *) m_data)[length] = 0; //Null terminator :-)

    running = false;

    return length;
}

int TAIAPIConnectEngineDevice::sendGetRequest(QJsonDocument & data, QString endpoint){
    uint8_t retval = 0;
    int port = m_preInitParams.getSubParamByName("Port")->getValue().toInt();
    QString addr = m_preInitParams.getSubParamByName("Address")->getValue();

    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    QUrl url;
    url.setHost(addr);
    url.setPort(port);
    url.setScheme("http");
    url.setPath("/" + endpoint);
    QNetworkRequest request(url);

    QNetworkReply* reply = mgr->get(request);

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QAbstractSocket::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QAbstractSocket::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(10000);   // 10 secs. timeout
    loop.exec();

    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "GET request to " << endpoint << "failed: " << reply->errorString();
        reply->deleteLater();
        return httpStatusCode;
    } else {
        // Print the response data
        QByteArray replyBuffer;
        replyBuffer = reply->readAll();
        data = QJsonDocument::fromJson(replyBuffer);
        reply->deleteLater();
        return httpStatusCode;
    }
}

bool TAIAPIConnectEngineDevice::getJsonArrayFromJsonDocumentField(QJsonArray & result, QJsonDocument & response, QString field) {
    if (!response.isObject()) {
        qDebug() << "Invalid JSON response";
        return false;
    }

    QJsonObject jsonObject = response.object();
    if (!jsonObject.contains(field)) {
        qDebug() << field << " field not found in JSON response";
        return false;
    }

    if (jsonObject[field].isArray()) result = jsonObject[field].toArray();
    else return false;

    return true;
}

bool TAIAPIConnectEngineDevice::getStringFromJsonDocumentField(QString & result, QJsonDocument & response, QString field) {
    if (!response.isObject()) {
        qDebug() << "Invalid JSON response";
        return false;
    }

    QJsonObject jsonObject = response.object();
    if (!jsonObject.contains(field)) {
        qDebug() << field << " field not found in JSON response";
        return false;
    }

    if (jsonObject[field].isString()) result = jsonObject[field].toString();
    else return false;

    if (result.startsWith("\"")) {
        result.remove(0, 1);
    }

    if (result.endsWith("\"")) {
        result.remove(result.length() - 1, 1);
    }

    return true;
}

uint8_t TAIAPIConnectEngineDevice::getServerMode() {
    //todo running
    QJsonDocument response;
    int statusCode = sendGetRequest(response, QString("get_mode"));
    if (statusCode != 200) return ENDPOINT_ERROR;

    QString responseString;
    bool ok = getStringFromJsonDocumentField(responseString, response, QString("mode"));
    if (!ok) {
        return ENDPOINT_ERROR;
    }

    if (responseString == "train") {
        return ENDPOINT_TRAIN;
    }

    if (responseString == "predict") {
        return ENDPOINT_PREDICT;
    }

    //todo return

    return ENDPOINT_ERROR;
}

bool TAIAPIConnectEngineDevice::setServerMode(uint8_t mode) {
    //todo running
    int statusCode = -1;
    if (mode == ENDPOINT_PREDICT) {
        QJsonDocument response;
        statusCode = sendGetRequest(response, QString("set_predict_mode"));
    } else if (mode == ENDPOINT_TRAIN) {
        QJsonDocument response;
        statusCode = sendGetRequest(response, QString("set_train_mode"));
    }

    if (statusCode != 200) {
        return false;
    } else {
        return true;
    }
}
