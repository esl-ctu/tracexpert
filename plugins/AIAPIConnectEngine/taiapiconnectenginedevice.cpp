#include "taiapiconnectenginedevice.h"

TAIAPIConnectEngineDevice::TAIAPIConnectEngineDevice(QString name, QString info) {
    m_name = name;
    m_info = info;

    m_preInitParams = TConfigParam("AIAPIConnectEngineDevice pre-init config", "", TConfigParam::TType::TDummy, "");
    m_postInitParams = TConfigParam("AIAPIConnectEngineDevice post-init config", "", TConfigParam::TType::TDummy, "");
    TConfigParam address = TConfigParam("Address", "127.0.0.1", TConfigParam::TType::TString, "Address to connect to (not a hostname)");
    TConfigParam port = TConfigParam("Port", "8000", TConfigParam::TType::TInt, "Port to connect to");
    TConfigParam inputSize = TConfigParam("Input size", "30000", TConfigParam::TType::TInt, "How big should ech individual input to the model be. Reserved for future use, not checked.", true);
    //TConfigParam separator = TConfigParam("Separator", "\n", TConfigParam::TType::TString, "Separator for individual values in the input. Defaults to newline.");
    auto type = TConfigParam("Data type", QString("int32_t"), TConfigParam::TType::TEnum, "");
    type.addEnumValue("int16_t");
    type.addEnumValue("int32_t");
    type.addEnumValue("int64_t");
    type.addEnumValue("unt16_t");
    type.addEnumValue("unt32_t");
    type.addEnumValue("unt64_t");
    type.addEnumValue("Double");

    m_preInitParams.addSubParam(address);
    m_preInitParams.addSubParam(port);
    m_preInitParams.addSubParam(inputSize);
    //m_preInitParams.addSubParam(separator);
    m_preInitParams.addSubParam(type);

    running = false;
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
    m_analOutputStreams.append(new TAIAPIConnectEngineDeviceOutputStream("First set", "Stream of traces in first set", [=](const uint8_t * buffer, size_t length){ return fillData(buffer, length); }));

    //todo test connect

    int port = m_preInitParams.getSubParamByName("Port")->getValue().toInt();
    QString addr = m_preInitParams.getSubParamByName("Address")->getValue();

    m_initialized = true;
}

void TAIAPIConnectEngineDevice::deInit(bool *ok /*= nullptr*/) {
    //todo remove streams and actions
    //m_initialized = false;
}

TConfigParam TAIAPIConnectEngineDevice::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TAIAPIConnectEngineDevice::setPostInitParams(TConfigParam params) {}

QList<TAnalAction *> TAIAPIConnectEngineDevice::getActions() const {}

QList<TAnalInputStream *> TAIAPIConnectEngineDevice::getInputDataStreams() const {}

QList<TAnalOutputStream *> TAIAPIConnectEngineDevice::getOutputDataStreams() const {}

bool TAIAPIConnectEngineDevice::isBusy() const {
    return !running;
}

int getTypeSize(QString type) {
    if (type == "int16_t") return sizeof(int16_t);
    if (type == "int32_t") return  sizeof(int32_t);
    if (type == "int64_t") return  sizeof(int64_t);
    if (type == "uint16_t") return  sizeof(uint16_t);
    if (type == "uint32_t") return  sizeof(uint32_t);
    if (type == "uint64_t") return  sizeof(uint64_t);
    if (type == "Double") return  sizeof(double);
    return 0;
}

bool TAIAPIConnectEngineDevice::analyzeData() {
    running = true;

    bool ok = true;
    int port = m_preInitParams.getSubParamByName("Port")->getValue().toInt();
    QString addr = m_preInitParams.getSubParamByName("Address")->getValue();
    addr += ":";
    addr += "addr";
    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    int inputItemSize = m_preInitParams.getSubParamByName("Input size")->getValue().toInt();
    uint8_t typeSize = getTypeSize(type);

    if (m_length % inputItemSize == 0) {
        qDebug("Uneven number of samples received");
        return false;
    }

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        return false;
    }


    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    const QUrl url(addr);
    QNetworkRequest request(url);

    QJsonObject param;
    for (int i = 0; i < m_length/inputItemSize; ++i)  {
        QString val;
        for (int j = 0; j < inputItemSize; ++j) {
            if (j) val += ",";
            if (type == "int16_t") val += QString::number(((int16_t *) m_data)[j]);
            if (type == "int32_t") val += QString::number(((int32_t *) m_data)[j]);
            if (type == "int64_t") val += QString::number(((int64_t *) m_data)[j]);
            if (type == "uint16_t") val += QString::number(((uint16_t *) m_data)[j]);
            if (type == "uint32_t") val += QString::number(((uint32_t *) m_data)[j]);
            if (type == "uint64_t") val += QString::number(((uint64_t *) m_data)[j]);
            if (type == "Double") val += QString::number(((double *) m_data)[j]);
        }
        param.insert("data", QJsonValue::fromVariant(val)); //todo asi by to mělo být array
    }

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = mgr->post(request, QJsonDocument(param).toJson(QJsonDocument::Compact));

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

    if (!ok){
        qDebug("Model reported invalid data");
    }

    QJsonDocument document = QJsonDocument::fromJson(replyBuffer);
    QJsonObject rootObj = document.object();
    //TODO



    running = false;
}

//todo ošetřit running i u streamů

size_t TAIAPIConnectEngineDevice::getData(uint8_t * buffer, size_t length) {
    if (!m_data)
        return 0;

    int i;

    for (i = 0; m_position < m_length && i < length; i++, m_position++) {
        buffer[i] = m_data[m_position];
    }

    return i;
}

size_t TAIAPIConnectEngineDevice::fillData(const uint8_t * buffer, size_t length) {
    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    int inputItemSize = m_preInitParams.getSubParamByName("Input size")->getValue().toInt();
    uint8_t typeSize = getTypeSize(type);

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        return false;
    }

    if (m_data) {
        if (type == "int16_t") delete (int16_t *) m_data;
        if (type == "int32_t") delete (int32_t *) m_data;
        if (type == "int64_t") delete (int64_t *) m_data;
        if (type == "uint16_t") delete (uint16_t *) m_data;
        if (type == "uint32_t") delete (uint32_t *) m_data;
        if (type == "uint64_t") delete (uint64_t *) m_data;
        if (type == "Double") delete (double *) m_data;
        m_data = nullptr;
    }

    m_length = length / typeSize;
    m_data = new int[m_length];

    if (type == "int16_t") m_data = (void *) new int16_t[m_length];
    if (type == "int32_t") m_data = (void *) new int32_t[m_length];
    if (type == "int64_t") m_data = (void *) new int64_t[m_length];
    if (type == "uint16_t") m_data = (void *) new uint16_t[m_length];
    if (type == "uint32_t") m_data = (void *) new uint32_t[m_length];
    if (type == "uint64_t") m_data = (void *) new uint64_t[m_length];
    if (type == "Double") m_data = (void *) new double[m_length];

    memcpy(m_data, buffer, length);

    return length;
}


