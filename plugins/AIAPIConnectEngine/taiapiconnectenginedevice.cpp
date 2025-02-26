#include "taiapiconnectenginedevice.h"

//todo:
/// upload - nefunguje víc jak jedna trace (opraveno pro text, ale ne pro int/...)
/// predict - nejspíš taky (taky opraveno pro text)
///
/// upload hdf5

bool validateParamD(QString in, double min, double max){
    bool ok;
    double val = in.toDouble(&ok);
    if (!ok) return false;

    if (val >= min && val <= max) return true;
        return false;
}

bool validateParamLL(QString in, long long min, long long max){
    bool ok;
    int val = in.toInt(&ok);
    if (!ok) return false;

    if (val >= min && val <= max) return true;
    return false;
}

bool processNumberInputIntoJSONArray(QJsonArray & jsonArray, void * dataIn, int inputItemSize, QString type, int len) {
    for (int i = 0; i < len; ++i)  {
        if (type == "int16_t") jsonArray.append(((int16_t *) dataIn)[i]);
        if (type == "int32_t") jsonArray.append(((int32_t *) dataIn)[i]);
        if (type == "int64_t") jsonArray.append(((int64_t *) dataIn)[i]);
        if (type == "uint16_t") jsonArray.append(((uint16_t *) dataIn)[i]);
        if (type == "uint32_t") {
            int64_t tmp = ((uint32_t *) dataIn)[i];
            jsonArray.append(tmp);
        }
        if (type == "uint64_t") {
            if (((uint64_t *) dataIn)[i] > INT64_MAX) {
                double tmp = ((uint64_t *) dataIn)[i];
                jsonArray.append(tmp);
            } else {
                int64_t tmp = ((uint64_t *) dataIn)[i];
                jsonArray.append(tmp);
            }

        }
        if (type == "Double") jsonArray.append(((double *) dataIn)[i]);
    }

    if (jsonArray.size() % inputItemSize != 0) {
        qDebug("Uneven number of samples received (3)");
        return false;
    }

    return true;
}

bool processTextInputIntoJSONArray(QJsonArray & jsonArray, void * dataIn, int inputItemSize) {
    char * data = (char *) dataIn;
    QString dataS(data);
    QStringList lines_in = dataS.split('\n', Qt::SkipEmptyParts);
    QStringList lines;

    static QRegularExpression regex("([+-]?[0-9]*[\\.][0-9]+|[+-]?[0-9]+)");
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
        return false;
    }

    bool ook = true;
    int loopIndex = 0;
    QJsonArray arrayToInsert;
    for (const QString &str : lines) {
        loopIndex++;

        bool oook;
        double number = str.toDouble(&oook);
        if (oook) arrayToInsert.append(number);
        else ook = false;

        if (loopIndex % inputItemSize == 0) {
            jsonArray.append(arrayToInsert);
            arrayToInsert = QJsonArray();
        }
    }

    if (!ook) {
        qDebug("Input data invalid");
        return false;
    }

    return true;
}


TAIAPIConnectEngineDevice::TAIAPIConnectEngineDevice(QString name, QString info) {
    m_name = name;
    m_info = info;
    m_initialized = false;
    running = true;

    m_preInitParams = TConfigParam("AIAPIConnectEngineDevice pre-init config", "", TConfigParam::TType::TDummy, "");
    m_postInitParams = TConfigParam("AIAPIConnectEngineDevice post-init config", "", TConfigParam::TType::TDummy, "");

    TConfigParam address = TConfigParam("Address", "127.0.0.1", TConfigParam::TType::TString, "Address to connect to (not a hostname)");
    TConfigParam port = TConfigParam("Port", "8000", TConfigParam::TType::TInt, "Port to connect to");
    TConfigParam timeout = TConfigParam("Server timeout", "10000", TConfigParam::TType::TInt, "How long to wait for a response from the server. In miliseconds.");
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
    m_preInitParams.addSubParam(timeout);
    //m_preInitParams.addSubParam(separator);
    m_preInitParams.addSubParam(type);

    TConfigParam inputSize = TConfigParam("Prediction input size", "30000", TConfigParam::TType::TInt, "How big should ech individual input to the model be.");

    TConfigParam trainParams = TConfigParam("Training params", "", TConfigParam::TType::TDummy, "");
    trainParams.addSubParam(TConfigParam("Epochs", "60", TConfigParam::TType::TInt, ""));
    trainParams.addSubParam(TConfigParam("Batch size", "16", TConfigParam::TType::TInt, ""));
    trainParams.addSubParam(TConfigParam("Trials", "100", TConfigParam::TType::TInt, ""));
    trainParams.addSubParam(TConfigParam("Optimalization", "false", TConfigParam::TType::TBool, ""));
    trainParams.addSubParam(TConfigParam("Destination model", "", TConfigParam::TType::TString, ""));
    TConfigParam loadedDataset = TConfigParam("Used datset", "", TConfigParam::TType::TEnum, "");
    loadedDataset.addEnumValue("None");
    trainParams.addSubParam(loadedDataset);

    TConfigParam trainStatus = TConfigParam("Training status", "", TConfigParam::TType::TDummy, "");
    trainStatus.addSubParam(TConfigParam("Running?", "", TConfigParam::TType::TBool, "", true));
    trainStatus.addSubParam(TConfigParam("Epoch", "", TConfigParam::TType::TInt, "", true));
    trainStatus.addSubParam(TConfigParam("Accuracy", "", TConfigParam::TType::TReal, "", true));
    trainStatus.addSubParam(TConfigParam("Loss", "", TConfigParam::TType::TReal, "", true));
    trainStatus.addSubParam(TConfigParam("Val. accuracy", "", TConfigParam::TType::TReal, "", true));
    trainStatus.addSubParam(TConfigParam("Val. loss", "", TConfigParam::TType::TReal, "", true));

    TConfigParam serverMode = TConfigParam("Server mode", "", TConfigParam::TType::TEnum, "");
    serverMode.addEnumValue("Train");
    serverMode.addEnumValue("Predict");

    TConfigParam datasets = TConfigParam("List of datasets", "", TConfigParam::TType::TDummy, "", true);

    TConfigParam loadedModel = TConfigParam("Used model", "", TConfigParam::TType::TEnum, "");
    loadedModel.addEnumValue("None");

    TConfigParam uploadParams = TConfigParam("Upload params", "", TConfigParam::TType::TDummy, "");
    uploadParams.addSubParam(TConfigParam("Dataset name", "", TConfigParam::TType::TString, ""));
    uploadParams.addSubParam(TConfigParam("Class no.", "0", TConfigParam::TType::TInt, ""));
    uploadParams.addSubParam(TConfigParam("Trace size", "30000", TConfigParam::TType::TInt, ""));

    m_postInitParams.addSubParam(inputSize);
    m_postInitParams.addSubParam(serverMode);
    m_postInitParams.addSubParam(loadedModel);
    m_postInitParams.addSubParam(trainParams);
    m_postInitParams.addSubParam(trainStatus);
    m_postInitParams.addSubParam(uploadParams);
    m_postInitParams.addSubParam(datasets);

    dataReadyPredict = false;
    dataReadyTrain = false;
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
    running = true;

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

    QString timeout = params.getSubParamByName("Server timeout")->getValue();
    if(!validateParamLL(timeout, 100, INT32_MAX)) {
        params.getSubParamByName("Server timeout")->setState(TConfigParam::TState::TError, "Timeout invalid. Expected value [100, INT32_MAX].");
        ok = false;
    }

    running = false;
    if (ok) {
        m_preInitParams = params;
        return m_preInitParams;
    } else {
        return params;
    }
}

void TAIAPIConnectEngineDevice::init(bool *ok /*= nullptr*/) {
    running = true;

    int timeout = m_preInitParams.getSubParamByName("Server timeout")->getValue().toInt();

    m_analActions.append(new TAIAPIConnectEngineDeviceAction("Analyze", "Runs the specified model on provided input data.", [=](){ analyzeData(); }));
    m_analActions.append(new TAIAPIConnectEngineDeviceAction("Upload data", "Runs the specified model on provided input data.", [=](){ uploadData(); }));
    m_analActions.append(new TAIAPIConnectEngineDeviceAction("Train", "Trains the model acording to set params.", [=](){ train(); }));
    //test model?
    m_analInputStreams.append(new TAIAPIConnectEngineDeviceInputStream("Prediction result", "Stream of prediction results", [=](uint8_t * buffer, size_t length){ return getData(buffer, length, false); }));
    m_analOutputStreams.append(new TAIAPIConnectEngineDeviceOutputStream("Prediction input", "Stream of traces as input to the model for prediction", [=](const uint8_t * buffer, size_t length){ return fillData(buffer, length, false); }));
    m_analInputStreams.append(new TAIAPIConnectEngineDeviceInputStream("Training result", "Stream of training results", [=](uint8_t * buffer, size_t length){ return getData(buffer, length, true); }));
    m_analOutputStreams.append(new TAIAPIConnectEngineDeviceOutputStream("Training input", "Stream of traces as input to a dataset for training", [=](const uint8_t * buffer, size_t length){ return fillData(buffer, length, true); }));

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
    timer.start(timeout);
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

    setServerMode(ENDPOINT_PREDICT);
    m_postInitParams = getPostInitParams();

    m_initialized = true;

    getServerMode();
    running = false;
}

void TAIAPIConnectEngineDevice::deInit(bool *ok /*= nullptr*/) {
    running = true;
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
    if (m_dataTrain) {
        if (type == "int16_t") delete[] (int16_t *) m_dataTrain;
        else if (type == "int32_t") delete[] (int32_t *) m_dataTrain;
        else if (type == "int64_t") delete[] (int64_t *) m_dataTrain;
        else if (type == "uint16_t") delete[] (uint16_t *) m_dataTrain;
        else if (type == "uint32_t") delete[] (uint32_t *) m_dataTrain;
        else if (type == "uint64_t") delete[] (uint64_t *) m_dataTrain;
        else if (type == "Double") delete[] (double *) m_dataTrain;
        else if (type == "Text") delete[] (char *) m_dataTrain;
        m_dataTrain = nullptr;
    }

    if (m_dataPredict) {
        if (type == "int16_t") delete[] (int16_t *) m_dataPredict;
        else if (type == "int32_t") delete[] (int32_t *) m_dataPredict;
        else if (type == "int64_t") delete[] (int64_t *) m_dataPredict;
        else if (type == "uint16_t") delete[] (uint16_t *) m_dataPredict;
        else if (type == "uint32_t") delete[] (uint32_t *) m_dataPredict;
        else if (type == "uint64_t") delete[] (uint64_t *) m_dataPredict;
        else if (type == "Double") delete[] (double *) m_dataPredict;
        else if (type == "Text") delete[] (char *) m_dataPredict;
        m_dataTrain = nullptr;
    }

    m_initialized = false;
    running = false;
}

TConfigParam TAIAPIConnectEngineDevice::getPostInitParams() const {
    bool ok = true;
    bool tRunning;
    int epoch, epochs, batchSize, trials;
    double accuracy, loss, valAccuracy, valLoss;
    QMap<QString, QMap<QString, QPair<int, int>>> datasetMap;
    QList<QString> modelList;

    uint8_t mode = getServerMode();
    if (mode == ENDPOINT_ERROR)
        ok = false;
    if (mode == ENDPOINT_TRAIN) {
        ok &= getTrainingStatus(tRunning, epoch, accuracy, loss, valAccuracy, valLoss);
        ok &= getTrainingParams(epochs, batchSize, trials);
    }
    ok &= getListOfDatasets(datasetMap);
    ok &= getListOfModels(modelList);

    auto ret = m_postInitParams;

    if (!ok) {
        ret.setState(TConfigParam::TState::TError, "Unable to get post init params!");
        return ret;
    }

    if (mode == ENDPOINT_TRAIN) {
        ret.getSubParamByName("Training params")->getSubParamByName("Epochs")->setValue(epochs);
        ret.getSubParamByName("Training params")->getSubParamByName("Batch size")->setValue(batchSize);
        ret.getSubParamByName("Training params")->getSubParamByName("Trials")->setValue(trials);

        ret.getSubParamByName("Training status")->getSubParamByName("Running?")->setValue(running);
        ret.getSubParamByName("Training status")->getSubParamByName("Epoch")->setValue(epoch);
        ret.getSubParamByName("Training status")->getSubParamByName("Accuracy")->setValue(accuracy);
        ret.getSubParamByName("Training status")->getSubParamByName("Loss")->setValue(loss);
        ret.getSubParamByName("Training status")->getSubParamByName("Val. accuracy")->setValue(valAccuracy);
        ret.getSubParamByName("Training status")->getSubParamByName("Val. loss")->setValue(valLoss);
    } else {
        //Training params remain

        ret.getSubParamByName("Training status")->getSubParamByName("Running?")->setValue(false);
        ret.getSubParamByName("Training status")->getSubParamByName("Epoch")->setValue(-1);
        ret.getSubParamByName("Training status")->getSubParamByName("Accuracy")->setValue(-1);
        ret.getSubParamByName("Training status")->getSubParamByName("Loss")->setValue(-1);
        ret.getSubParamByName("Training status")->getSubParamByName("Val. accuracy")->setValue(-1);
        ret.getSubParamByName("Training status")->getSubParamByName("Val. loss")->setValue(-1);
    }

    if (mode == ENDPOINT_PREDICT)
        ret.getSubParamByName("Server mode")->setValue("Predict");
    if (mode == ENDPOINT_TRAIN)
        ret.getSubParamByName("Server mode")->setValue("Train");

    ret.getSubParamByName("List of datasets")->clearSubParams();
    ret.getSubParamByName("Training params")->getSubParamByName("Used datset")->clearEnumValues();
    ret.getSubParamByName("Training params")->getSubParamByName("Used datset")->addEnumValue("None");
    for (auto it = datasetMap.constBegin(); it != datasetMap.constEnd(); ++it) {
        TConfigParam tmpOut = TConfigParam(it.key(), "", TConfigParam::TType::TDummy, "", true);
        auto innerMap = it.value();
         for (auto itt = innerMap.constBegin(); itt != innerMap.constEnd(); ++itt) {
            TConfigParam tmpIn = TConfigParam(itt.key(), "", TConfigParam::TType::TDummy, "", true);
            TConfigParam numTraces = TConfigParam("Num. traces", "", TConfigParam::TType::TInt, "", true);
            TConfigParam shape = TConfigParam("Shape", "", TConfigParam::TType::TInt, "", true);
            numTraces.setValue(itt.value().first);
            shape.setValue(itt.value().second);

            tmpIn.addSubParam(numTraces);
            tmpIn.addSubParam(shape);

            tmpOut.addSubParam(tmpIn);
        }
        ret.getSubParamByName("List of datasets")->addSubParam(tmpOut);
        ret.getSubParamByName("Training params")->getSubParamByName("Used datset")->addEnumValue(it.key());
    }

    ret.getSubParamByName("Used model")->clearEnumValues();
    ret.getSubParamByName("Used model")->addEnumValue("None");
    for (auto it = modelList.begin(); it != modelList.end(); ++it) {
        ret.getSubParamByName("Used model")->addEnumValue(*it);
    }

    //Upload params remain

    return ret;
}

TConfigParam TAIAPIConnectEngineDevice::setPostInitParams(TConfigParam params) {
    bool ok = true;
    QString mode = params.getSubParamByName("Server mode")->getValue();

    if (mode == "Predict")
        ok &= setServerMode(ENDPOINT_PREDICT);
    else if (mode == "Train")
        ok &= setServerMode(ENDPOINT_TRAIN);
    else
        ok = false;

    if (mode == "Train") {
        if(!validateParamLL(params.getSubParamByName("Training params")->getSubParamByName("Epochs")->getValue(), 0, 255)){
            params.getSubParamByName("Training params")->getSubParamByName("Epochs")->setState(TConfigParam::TState::TWarning, "Expected value: [0, 255]");
            ok = false;
        } else {
            params.getSubParamByName("Training params")->getSubParamByName("Epochs")->resetState();
        }

        if(!validateParamLL(params.getSubParamByName("Training params")->getSubParamByName("Batch size")->getValue(), 0, 255)){
            params.getSubParamByName("Training params")->getSubParamByName("Batch size")->setState(TConfigParam::TState::TWarning, "Expected value: [0, 255]");
            ok = false;
        } else {
            params.getSubParamByName("Training params")->getSubParamByName("Batch size")->resetState();
        }

        if(!validateParamLL(params.getSubParamByName("Training params")->getSubParamByName("Trials")->getValue(), 0, 255)){
            params.getSubParamByName("Training params")->getSubParamByName("Trials")->setState(TConfigParam::TState::TWarning, "Expected value: [0, 255]");
            ok = false;
        } else {
            params.getSubParamByName("Training params")->getSubParamByName("Trials")->resetState();
        }

        QMap<QString, QMap<QString, QPair<int, int>>> datasetMap;
        ok &= getListOfDatasets(datasetMap);
        bool exists = false;
        QString datasetName = params.getSubParamByName("Training params")->getSubParamByName("Used datset")->getValue();

        for (auto it = datasetMap.constBegin(); it != datasetMap.constEnd(); ++it) {
            if(it.key() == datasetName) {
                exists = true;
                break;
            }
        }

        if (!exists && datasetName != "None") {
            params.getSubParamByName("Training params")->getSubParamByName("Used datset")->setState(TConfigParam::TState::TWarning, "Dataset does NOT exist!");
        }

        if (!ok) {
            m_postInitParams = params;
            m_postInitParams.setState(TConfigParam::TState::TError, "Invalid values in params, nothing was set!");
            return m_postInitParams;
        }

        if (datasetName != "None") {
            ok = loadDataset(datasetName);
            if (!ok){
                params.getSubParamByName("Training params")->getSubParamByName("Used datset")->setState(TConfigParam::TState::TWarning, "Error loading dataset (but the name is correct).");
            } else {
                m_postInitParams.getSubParamByName("Training params")->getSubParamByName("Used datset")->setValue(params.getSubParamByName("Training params")->getSubParamByName("Used datset")->getValue());
            }
        }


        int epochs = params.getSubParamByName("Training params")->getSubParamByName("Epochs")->getValue().toInt();
        int batchSize = params.getSubParamByName("Training params")->getSubParamByName("Batch size")->getValue().toInt();
        int trials = params.getSubParamByName("Training params")->getSubParamByName("Trials")->getValue().toInt();
        ok = setTrainParams(epochs, batchSize, trials);
    } else if (mode == "Predict") {
        QList<QString> modelList;
        ok &= getListOfModels(modelList);
        bool exists = false;
        QString modelName = params.getSubParamByName("Used model")->getValue();

        for (auto it = modelList.constBegin(); it != modelList.constEnd(); ++it) {
            if(*it == modelName) {
                exists = true;
                break;
            }
        }

        if (!exists && modelName != "None") {
            params.getSubParamByName("Used model")->setState(TConfigParam::TState::TWarning, "Model does NOT exist!");
        }

        if (!ok) {
            m_postInitParams = params;
            m_postInitParams.setState(TConfigParam::TState::TError, "Invalid values in params, nothing was set!");
            return m_postInitParams;
        }

        if (modelName != "None") {
            bool opti = params.getSubParamByName("Training params")->getSubParamByName("Optimalization")->getValue() == "true";
            ok = loadModel(modelName, opti);
            if (!ok) {
                params.getSubParamByName("Used model")->setState(TConfigParam::TState::TWarning, "Error loading model.");
            } else {
                m_postInitParams.getSubParamByName("Used model")->setValue(params.getSubParamByName("Used model")->getValue());
            }
        }

        if(!validateParamLL(params.getSubParamByName("Prediction input size")->getValue(), 1, INT32_MAX)){
            params.getSubParamByName("Prediction input size")->setState(TConfigParam::TState::TWarning, "Expected value: [1, INT32_MAX]");
            ok = false;
        } else {
            params.getSubParamByName("Prediction input size")->resetState();
        }

    }

    //Upload params:
    auto prm1 = params.getSubParamByName("Upload params")->getSubParamByName("Dataset name")->getValue();
    m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Dataset name")->setValue(prm1);

    if(!validateParamLL(params.getSubParamByName("Upload params")->getSubParamByName("Class no.")->getValue(), 0, 255)){
        params.getSubParamByName("Upload params")->getSubParamByName("Class no.")->setState(TConfigParam::TState::TWarning, "Expected value: [0, 255]");
        ok = false;
    } else {
        m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Class no.")->resetState();
        auto prm2 = params.getSubParamByName("Upload params")->getSubParamByName("Class no.")->getValue();
        m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Class no.")->setValue(prm2);
    }

    if(!validateParamLL(params.getSubParamByName("Upload params")->getSubParamByName("Trace size")->getValue(), 1, INT32_MAX)){
        params.getSubParamByName("Upload params")->getSubParamByName("Trace size")->setState(TConfigParam::TState::TWarning, "Expected value: [1, INT32_MAX]");
        ok = false;
    } else {
        m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Trace size")->resetState();
        auto prm3 = params.getSubParamByName("Upload params")->getSubParamByName("Trace size")->getValue();
        m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Trace size")->setValue(prm3);
    }

    //Final
    if (ok) {
        m_postInitParams.resetState(true);
        m_postInitParams = getPostInitParams();
    } else {
        m_postInitParams = params;
        m_postInitParams.setState(TConfigParam::TState::TError, "Unable to set postinit params!");
    }

    return m_postInitParams;
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



bool TAIAPIConnectEngineDevice::uploadData() { //upload_data endpoint
    if (running) {
        return false;
    }
    running = true;
    bool ok;

    QString dataset = m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Dataset name")->getValue();
    int classId = m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Class no.")->getValue().toInt();
    int traceSize = m_postInitParams.getSubParamByName("Upload params")->getSubParamByName("Trace size")->getValue().toInt();

    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    uint8_t typeSize = getTypeSize(type);

    if (m_lengthTrain % traceSize == 0 && type != "Text") {
        qDebug("Uneven number of samples received (1)");
        running = false;
        return false;
    }

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        running = false;
        return false;
    }

    QJsonArray jsonArray;

    if (type == "Text") {
        ok = processTextInputIntoJSONArray(jsonArray, m_dataTrain, traceSize);
        if (!ok) {
            running = false;
            return false;
        }
    } else {
        ok = processNumberInputIntoJSONArray(jsonArray, m_dataTrain, traceSize, type, m_lengthTrain);
        if (!ok) {
            running = false;
            return false;
        }
    }

    QJsonObject param;
    param["traces"] = jsonArray;
    param["traces_class"] = classId;
    param["dataset_name"] = dataset;

    int okval = 1;
    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(param, jsonResponse, QString("upload_data"), true);
    if (statusCode != 200) {
        qDebug() << "POST request failed with code " << statusCode;
        okval = 0;
    }

    QString message = "Failed";
    if (okval) {
        QJsonObject jsonObject = jsonResponse.object();
        if (!jsonObject.contains("message")) {
            qDebug() << "Message field not found in JSON response";
            okval = 0;
        } else {
            message = jsonObject.value("message").toString();
        }
    }

    QByteArray messageBytes = message.toUtf8();
    if (type == "Text") {
        if (m_dataTrain) delete[] (char*) m_dataTrain;
        m_dataTrain = (void*) new char[message.length() + 1];
        memcpy(m_dataTrain, messageBytes, message.length());
        ((char*) m_dataTrain)[message.length()] = 0;
    } else {
        if (type == "int16_t") {
            if (m_dataTrain) delete[] (int16_t*) m_dataTrain;
            m_dataTrain = (void*) new int16_t[1];
            ((int16_t *) m_dataTrain)[0] = okval;
        }
        if (type == "int32_t") {
            if (m_dataTrain) delete[] (int32_t*) m_dataTrain;
            m_dataTrain = (void*) new int32_t[1];
            ((int32_t *) m_dataTrain)[0] = okval;
        }
        if (type == "int64_t")  {
            if (m_dataTrain) delete[] (int64_t*) m_dataTrain;
            m_dataTrain = (void*) new int64_t[1];
            ((int64_t *) m_dataTrain)[0] = okval;
        }
        if (type == "uint16_t") {
            if (m_dataTrain) delete[] (uint16_t*) m_dataTrain;
            m_dataTrain = (void*) new uint16_t[1];
            ((uint16_t *) m_dataTrain)[0] = okval;
        }
        if (type == "uint32_t") {
            if (m_dataTrain) delete[] (uint32_t*) m_dataTrain;
            m_dataTrain = (void*) new uint32_t[1];
            ((uint32_t *) m_dataTrain)[0] = okval;
        }
        if (type == "uint64_t") {
            if (m_dataTrain) delete[] (uint64_t*) m_dataTrain;
            m_dataTrain = (void*) new uint64_t[1];
            ((uint64_t *) m_dataTrain)[0] = okval;
        }
        if (type == "Double") {
            if (m_dataTrain) delete[] (double*) m_dataTrain;
            m_dataTrain = (void*) new double[1];
            ((double *) m_dataTrain)[0] = okval;
        }
        m_lengthPredict = 1;
    }

    running = false;
    return false;

    dataReadyTrain = true;
    running = false;
    return true;
}

bool TAIAPIConnectEngineDevice::analyzeData() { //predict endpoint
    //todo output can't be longer than input (does it matter??)
    if (running) {
        return false;
    }
    running = true;

    setServerMode(ENDPOINT_PREDICT);

    dataReadyPredict = false;

    bool ok = true;
    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    int inputItemSize = m_postInitParams.getSubParamByName("Prediction input size")->getValue().toInt();
    uint8_t typeSize = getTypeSize(type);

    if (m_lengthPredict % inputItemSize == 0 && type != "Text") {
        qDebug("Uneven number of samples received (1)");
        running = false;
        return false;
    }

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        running = false;
        return false;
    }

    QJsonArray jsonArray;

    if (type == "Text") {
        ok = processTextInputIntoJSONArray(jsonArray, m_dataPredict, inputItemSize);
        if (!ok) {
            running = false;
            return false;
        }
    } else {
        ok = processNumberInputIntoJSONArray(jsonArray, m_dataPredict, inputItemSize, type, m_lengthPredict);
        if (!ok) {
            running = false;
            return false;
        }
    }

    QJsonObject param;
    param["data"] = jsonArray;

    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(param, jsonResponse, QString("predict"), true);
    if (statusCode != 200) {
        qDebug() << "POST request failed";
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
        if (m_lengthPredict < max) max = m_lengthPredict;
        std::memcpy(m_dataPredict, cStr, max);
        m_lengthPredict = max;
    } else {
        size_t i = 0;
        for (const QJsonValue &value : predictionArray) {
            if (i == m_lengthPredict) break;
            if (type == "int16_t") ((int16_t *) m_dataPredict)[i] = value.toInt();
            if (type == "int32_t") ((int32_t *) m_dataPredict)[i] = value.toInt();
            if (type == "int64_t") ((int64_t *) m_dataPredict)[i] = value.toInt();
            if (type == "uint16_t") ((uint16_t *) m_dataPredict)[i] = value.toInt();
            if (type == "uint32_t") ((uint32_t *) m_dataPredict)[i] = value.toInt();
            if (type == "uint64_t") ((uint64_t *) m_dataPredict)[i] = value.toInt();
            if (type == "Double") ((double *) m_dataPredict)[i] = value.toDouble();
            i++;
        }
        m_lengthPredict = i;
    }

    dataReadyPredict = true;
    running = false;

    return true;
}

bool TAIAPIConnectEngineDevice::stopTraining() {
    QJsonObject json;
    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(json, jsonResponse, QString("stop_training"));
    if (statusCode != 200) {
        qDebug() << "POST request failed";
        running = false;
        return false;
    }

    return true;

}

bool TAIAPIConnectEngineDevice::loadModel(QString name, bool opti/* = false*/) {
    QJsonObject json;
    json["model_name"] = name;
    json["model_optimization"] = opti;

    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(json, jsonResponse, QString("load_model"));
    if (statusCode != 200) {
        qDebug() << "POST request failed";
        running = false;
        return false;
    }

    return true;

}

bool TAIAPIConnectEngineDevice::deleteDataset(QString name, int fromTime/* = 0*/, int toTime/* = 0*/) {
    QJsonObject json;
    json["dataset_name"] = name;
    json["from_time"] = fromTime;
    json["to_time"] = toTime;

    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(json, jsonResponse, QString("delete_dataset"));
    if (statusCode != 200) {
        qDebug() << "POST request failed with code " << statusCode;
        running = false;
        return false;
    }

    return true;
}

bool TAIAPIConnectEngineDevice::train() {
    QString destinationModel = m_postInitParams.getSubParamByName("Training params")->getSubParamByName("Destination model")->getValue();
    bool optim = true;
    if(m_postInitParams.getSubParamByName("Optimalization")->getSubParamByName("Destination model")->getValue() == "false")
        optim = false;

    if (destinationModel == "")
        return false;

    QJsonObject json;
    json["model_name"] = destinationModel;
    json["model_optimization"] = optim;

    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(json, jsonResponse, QString("train"));
    if (statusCode != 200) {
        qDebug() << "POST request failed with code " << statusCode;
        running = false;
        return false;
    }

    return true;

}

bool TAIAPIConnectEngineDevice::setTrainParams(int epochs/* = 0*/, int batchSize/* = 0*/, int optimizationNumOfTrials/* = 0*/) {
    QJsonObject json;
    json["epochs"] = epochs;
    json["batch_size"] = batchSize;
    json["optimization_num_of_trials"] = optimizationNumOfTrials;

    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(json, jsonResponse, QString("set_train_params"));
    if (statusCode != 200) {
        qDebug() << "POST request failed";
        running = false;
        return false;
    }

    return true;
}

bool TAIAPIConnectEngineDevice::loadDataset(QString name, int fromTime/* = 0*/, int toTime/* = 0*/) {
    QJsonObject json;
    json["dataset_name"] = name;
    json["from_time"] = fromTime;
    json["to_time"] = toTime;

    QJsonDocument jsonResponse;
    int statusCode = sendPostRequest(json, jsonResponse, QString("load_dataset"));
    if (statusCode != 200) {
        qDebug() << "POST request failed";
        running = false;
        return false;
    }

    return true;
}

size_t TAIAPIConnectEngineDevice::getData(uint8_t * buffer, size_t length, bool train) {
    //train == true -> use m_dataTrain, otherwise m_dataPredict
    if (running) {
        return 0;
    }
    running = true;

    if (train) {
        if (!dataReadyTrain) {
            running = false;
            return 0;
        }
    } else {
        if (!dataReadyPredict) {
            running = false;
            return 0;
        }
    }

    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    uint8_t typeSize = getTypeSize(type);

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        running = false;
        return 0;
    }

    if (train) {
        if (!m_dataTrain) {
            running = false;
            return 0;
        }
    } else {
        if (!m_dataPredict) {
            running = false;
            return 0;
        }
    }

    size_t max = length;
    if (train) {
        if (length > typeSize*m_lengthTrain) max = typeSize*m_lengthTrain;
        memcpy(buffer, m_dataTrain, max);
    } else {
        if (length > typeSize*m_lengthPredict) max = typeSize*m_lengthPredict;
       memcpy(buffer, m_dataPredict, max);
    }


    running = false;

    return max;
}

size_t TAIAPIConnectEngineDevice::fillData(const uint8_t * buffer, size_t length, bool train) {
    //train == true -> use m_dataTrain, otherwise m_dataPredict
    if (running) {
        return 0;
    }
    running = true;

    if (train) {
        dataReadyTrain = false;
    } else {
        dataReadyPredict = false;
    }

    QString type = m_preInitParams.getSubParamByName("Data type")->getValue();
    uint8_t typeSize = getTypeSize(type);

    if (typeSize == 0) {
        qDebug("Invalid type of input");
        running = false;
        return 0;
    }

    if (train) {
        if (m_dataTrain) {
            if (type == "int16_t") delete[] (int16_t *) m_dataTrain;
            else if (type == "int32_t") delete[] (int32_t *) m_dataTrain;
            else if (type == "int64_t") delete[] (int64_t *) m_dataTrain;
            else if (type == "uint16_t") delete[] (uint16_t *) m_dataTrain;
            else if (type == "uint32_t") delete[] (uint32_t *) m_dataTrain;
            else if (type == "uint64_t") delete[] (uint64_t *) m_dataTrain;
            else if (type == "Double") delete[] (double *) m_dataTrain;
            else if (type == "Text") delete[] (char *) m_dataTrain;
            m_dataTrain = nullptr;
        }

        m_lengthTrain = length / typeSize;

        if (type == "int16_t") m_dataTrain = (void *) new int16_t[m_lengthTrain];
        else if (type == "int32_t") m_dataTrain = (void *) new int32_t[m_lengthTrain];
        else if (type == "int64_t") m_dataTrain = (void *) new int64_t[m_lengthTrain];
        else if (type == "uint16_t") m_dataTrain = (void *) new uint16_t[m_lengthTrain];
        else if (type == "uint32_t") m_dataTrain = (void *) new uint32_t[m_lengthTrain];
        else if (type == "uint64_t") m_dataTrain = (void *) new uint64_t[m_lengthTrain];
        else if (type == "Double") m_dataTrain = (void *) new double[m_lengthTrain];
        else if (type == "Text") m_dataTrain = (void *) new char[m_lengthTrain + 1];
        else {
            qDebug("Invalid type of input (2)");
            running = false;
            return 0;
        }

        memcpy(m_dataTrain, buffer, length);
        if (type == "Text") ((char *) m_dataTrain)[length] = 0; //Null terminator :-)
    } else {
        if (m_dataPredict) {
            if (type == "int16_t") delete[] (int16_t *) m_dataPredict;
            else if (type == "int32_t") delete[] (int32_t *) m_dataPredict;
            else if (type == "int64_t") delete[] (int64_t *) m_dataPredict;
            else if (type == "uint16_t") delete[] (uint16_t *) m_dataPredict;
            else if (type == "uint32_t") delete[] (uint32_t *) m_dataPredict;
            else if (type == "uint64_t") delete[] (uint64_t *) m_dataPredict;
            else if (type == "Double") delete[] (double *) m_dataPredict;
            else if (type == "Text") delete[] (char *) m_dataPredict;
            m_dataPredict = nullptr;
        }

        m_lengthPredict = length / typeSize;

        if (type == "int16_t") m_dataPredict = (void *) new int16_t[m_lengthPredict];
        else if (type == "int32_t") m_dataPredict = (void *) new int32_t[m_lengthPredict];
        else if (type == "int64_t") m_dataPredict = (void *) new int64_t[m_lengthPredict];
        else if (type == "uint16_t") m_dataPredict = (void *) new uint16_t[m_lengthPredict];
        else if (type == "uint32_t") m_dataPredict = (void *) new uint32_t[m_lengthPredict];
        else if (type == "uint64_t") m_dataPredict = (void *) new uint64_t[m_lengthPredict];
        else if (type == "Double") m_dataPredict = (void *) new double[m_lengthPredict];
        else if (type == "Text") m_dataPredict = (void *) new char[m_lengthPredict + 1];
        else {
            qDebug("Invalid type of input (2)");
            running = false;
            return 0;
        }

        memcpy(m_dataPredict, buffer, length);
        if (type == "Text") ((char *) m_dataPredict)[length] = 0; //Null terminator :-)
    }

    running = false;

    return length;
}

int TAIAPIConnectEngineDevice::sendPostRequest(QJsonObject & in, QJsonDocument & out, QString endpoint, bool ignoreRunningState/* = false*/) const {
    if (running && !ignoreRunningState) {
        qDebug() << "Busy! Can't POST";
        return -1;
    }
    bool* tmpR = (bool*) &running;
    *tmpR = true;

    uint8_t retval = 0;
    auto prms = m_preInitParams;
    int port = prms.getSubParamByName("Port")->getValue().toInt();
    QString addr = prms.getSubParamByName("Address")->getValue();
    int timeout = prms.getSubParamByName("Server timeout")->getValue().toInt();
    bool ok = true;

    QNetworkAccessManager *mgr = new QNetworkAccessManager();
    QUrl url;
    url.setHost(addr);
    url.setPort(port);
    url.setScheme("http");
    url.setPath(QString("/") + endpoint);
    QNetworkRequest request(url);

    QJsonDocument doc(in);
    QByteArray data = doc.toJson(QJsonDocument::Compact);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = mgr->post(request, data);

    QTimer timer;
    timer.setSingleShot(true);

    QEventLoop loop;
    QAbstractSocket::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    QAbstractSocket::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(timeout);
    loop.exec();
    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

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

    *tmpR = false;

    if (!ok){
        qDebug("POST request reported invalid data");
        return httpStatusCode;
    }

    out = QJsonDocument::fromJson(replyBuffer);

    if (!out.isObject()) {
        qDebug() << "Invalid JSON response";
        return httpStatusCode;
    }

    return httpStatusCode;
}

int TAIAPIConnectEngineDevice::sendGetRequest(QJsonDocument & data, QString endpoint) const {
    if (running) {
        return -1;
    }
    bool* tmpR = (bool*) &running;
    *tmpR = true;

    uint8_t retval = 0;
    auto prms = m_preInitParams;
    int port = prms.getSubParamByName("Port")->getValue().toInt();
    QString addr = prms.getSubParamByName("Address")->getValue();
    int timeout = prms.getSubParamByName("Server timeout")->getValue().toInt();

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
    timer.start(timeout);
    loop.exec();  

    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "GET request to " << endpoint << "failed: " << reply->errorString();
        reply->deleteLater();
        *tmpR = false;
        return httpStatusCode;
    } else {
        // Print the response data
        QByteArray replyBuffer;
        replyBuffer = reply->readAll();
        data = QJsonDocument::fromJson(replyBuffer);
        reply->deleteLater();
        *tmpR = false;
        return httpStatusCode;
    }
}

bool TAIAPIConnectEngineDevice::getJsonArrayFromJsonDocumentField(QJsonArray & result, QJsonDocument & response, QString field) const {
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

bool TAIAPIConnectEngineDevice::getJsonArrayFromJsonObject(QJsonArray & result, QJsonObject & obj, QString field) const {
    if (obj.contains(field) && obj[field].isArray()) {
        result = obj["datasets"].toArray();
        return true;
    }

    return false;
}

bool TAIAPIConnectEngineDevice::getStringFromJsonDocumentField(QString & result, QJsonDocument & response, QString field) const {
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

bool TAIAPIConnectEngineDevice::getBoolFromJsonDocumentField(bool & result, QJsonDocument & response, QString field) const {
    if (!response.isObject()) {
        qDebug() << "Invalid JSON response";
        return false;
    }

    QJsonObject jsonObject = response.object();
    if (!jsonObject.contains(field)) {
        qDebug() << field << " field not found in JSON response";
        return false;
    }

    if (jsonObject[field].isBool()) result = jsonObject[field].toBool();
    else return false;

    return true;
}

bool TAIAPIConnectEngineDevice::getIntFromJsonDocumentField(int & result, QJsonDocument & response, QString field) const {
    if (!response.isObject()) {
        qDebug() << "Invalid JSON response";
        return false;
    }

    QJsonObject jsonObject = response.object();
    if (!jsonObject.contains(field)) {
        qDebug() << field << " field not found in JSON response";
        return false;
    }

    if (!jsonObject[field].isNull()) result = jsonObject[field].toInt();
    else return false;

    return true;
}

bool TAIAPIConnectEngineDevice::getDoubleFromJsonDocumentField(double & result, QJsonDocument & response, QString field) const {
    if (!response.isObject()) {
        qDebug() << "Invalid JSON response";
        return false;
    }

    QJsonObject jsonObject = response.object();
    if (!jsonObject.contains(field)) {
        qDebug() << field << " field not found in JSON response";
        return false;
    }

    if (jsonObject[field].isDouble()) result = jsonObject[field].toDouble();
    else return false;

    return true;
}

uint8_t TAIAPIConnectEngineDevice::getServerMode() const {
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

    return ENDPOINT_ERROR;
}

bool TAIAPIConnectEngineDevice::setServerMode(uint8_t mode) const {
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

bool TAIAPIConnectEngineDevice::getTrainingStatus(bool & running, int & epoch, double & accuracy, double & loss, double & valAccuracy, double & valLoss) const {
    QJsonDocument response;
    int statusCode = sendGetRequest(response, QString("get_training_progress"));
    if (statusCode != 200) return false;

    bool responseBool;
    bool ok = getBoolFromJsonDocumentField(responseBool, response, QString("train_in_progress"));
    if (!ok) {
        return false;
    } else {
        running = responseBool;
    }

    if (!responseBool) {
        running = false;
        epoch = -1;
        accuracy = -1;
        loss = -1;
        valAccuracy = -1;
        valLoss = -1;
        return true;
    }

    int responseInt;
    ok = getIntFromJsonDocumentField(responseInt, response, QString("epoch"));
    if (ok) {
        epoch = responseInt;
    } else {
        epoch = -1;
    }

    double responseDouble;
    ok = getDoubleFromJsonDocumentField(responseDouble, response, QString("accuracy"));
    if (ok) {
        accuracy = responseDouble;
    } else {
        accuracy = -1;
    }

    ok = getDoubleFromJsonDocumentField(responseDouble, response, QString("loss"));
    if (ok) {
        loss = responseDouble;
    } else {
        loss = -1;
    }

    ok = getDoubleFromJsonDocumentField(responseDouble, response, QString("val_accuracy"));
    if (ok) {
        valAccuracy = responseDouble;
    } else {
        valAccuracy = -1;
    }

    ok = getDoubleFromJsonDocumentField(responseDouble, response, QString("val_loss"));
    if (ok) {
        valLoss = responseDouble;
    } else {
        valLoss = -1;
    }

    return true;
}

bool TAIAPIConnectEngineDevice::getTrainingParams(int & epochs, int & batchSize, int & trials) const {
    QJsonDocument response;
    int statusCode = sendGetRequest(response, QString("get_training_params"));
    if (statusCode != 200) return false;

    int responseInt;
    bool ok = getIntFromJsonDocumentField(responseInt, response, QString("epochs"));
    if (ok) {
        epochs = responseInt;
    } else {
        return false;
    }

    ok = getIntFromJsonDocumentField(responseInt, response, QString("batch_size"));
    if (ok) {
        batchSize = responseInt;
    } else {
        return false;
    }

    ok = getIntFromJsonDocumentField(responseInt, response, QString("optimization_num_of_trials"));
    if (ok) {
        trials = responseInt;
    } else {
        return false;
    }

    return true;

}

bool TAIAPIConnectEngineDevice::getListOfModels(QList<QString> & modelList) const {
    QJsonDocument response;
    int statusCode = sendGetRequest(response, QString("list_models"));
    if (statusCode != 200) return false;

    QJsonArray models;
    bool ok = getJsonArrayFromJsonDocumentField(models, response, QString("message"));
    if (!ok) return false;

    modelList = QList<QString>();

    for (int i = 0; i < models.size(); ++i) {
        const QJsonValue& value = models[i];
        if (value.isString()) {
            modelList.append(value.toString());
        } else {
            qDebug("Model name from JSON is not a string");
            return false;
        }
    }

    return true;
}

bool TAIAPIConnectEngineDevice::getListOfDatasets(QMap<QString, QMap<QString, QPair<int, int>>> & datasetMap) const {
    QJsonDocument response;
    int statusCode = sendGetRequest(response, QString("list_datasets"));
    if (statusCode != 200) return false;

    QJsonArray datasets;
    bool ok = getJsonArrayFromJsonDocumentField(datasets, response, QString("files"));
    if (!ok) return false;

    // Loop through each file
    for (int i = 0; i < datasets.size(); ++i) {
        QJsonObject fileObject = datasets[i].toObject();

        // Get the dataset_name
        QString responseString;
        if (fileObject.contains("dataset_name")) {
            responseString = fileObject.value("dataset_name").toString();
        } else {
            qDebug("No datasets are available");
            return true;
        }

        // Get the datasets array
        QJsonArray datasetsArray;
        if (getJsonArrayFromJsonObject(datasetsArray, fileObject, QString("datasets"))) {

            // Loop through each dataset
            QMap<QString, QPair<int, int>> currMap;
            for (int j = 0; j < datasetsArray.size(); ++j) {
                QJsonObject datasetObject = datasetsArray[j].toObject();

                // Extract key, number of traces, and trace shape
                if (datasetObject.contains("key") && datasetObject.contains("number of traces") && datasetObject.contains("trace shape")) {
                    QString key = datasetObject["key"].toString();
                    int numberOfTraces = datasetObject["number of traces"].toInt();
                    int traceShapeSize = datasetObject["trace shape"].toArray().at(0).toInt();  //WARNING, just the first index!

                    // Store the data in the QMap, using dataset name, number of traces, and trace shape size
                    currMap.insert(key, qMakePair(numberOfTraces, traceShapeSize));
                } else {
                    return false;
                }
                datasetMap.insert(responseString, currMap);
            }
        } else {
            return false;
        }
    }

    return true;
}
