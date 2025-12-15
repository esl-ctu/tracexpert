#ifndef TAIAPICONNECTENGINEDEVICE_H
#define TAIAPICONNECTENGINEDEVICE_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrl>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonArray>
#include <QMutex>
#include <QVariant>
#include <QRegularExpression>
#include <QFile>
#include <limits>
#include <hdf5.h>

#include "tconfigparam.h"
#include "tanaldevice.h"
#include "taiapiconnectenginedeviceaction.h"
#include "taiapiconnectenginedeviceinputstream.h"
#include "taiapiconnectenginedeviceoutputstream.h"

class TAIAPIConnectEngineDevice : public TAnalDevice {
public:
    TAIAPIConnectEngineDevice(QString name, QString info);
    virtual ~TAIAPIConnectEngineDevice();

    /// AnalDevice name
    virtual QString getName() const override;
    /// AnalDevice info
    virtual QString getInfo() const override;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const override;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    /// Initialize the analytic device
    virtual void init(bool *ok = nullptr) override;
    /// Deinitialize the analytic device
    virtual void deInit(bool *ok = nullptr) override;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const override;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    /// Get list of available actions
    virtual QList<TAnalAction *> getActions() const override;

    /// Get list of available input data streams
    virtual QList<TAnalInputStream *> getInputDataStreams() const override;
    /// Get list of available output data streams
    virtual QList<TAnalOutputStream *> getOutputDataStreams() const override;

    virtual bool isBusy() const override;

    bool analyzeData();
    bool uploadData();
    bool uploadHDF5();

    //testModel
    size_t getData(uint8_t * buffer, size_t length, bool train);
    size_t fillData(const uint8_t * buffer, size_t length, bool train);

    //size_t fillData(const uint8_t * buffer, size_t length, QList<QList<uint8_t> *> & set);
    //void processData(bool subtract);
    //

private:
    //size_t m_traceLength;
    bool m_initialized;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    QString m_name;
    QString m_info;

    QList<TAnalAction *> m_analActions;

    QList<TAnalInputStream *> m_analInputStreams;
    QList<TAnalOutputStream *> m_analOutputStreams;

    size_t m_lengthTrain; //in sizeof(type)
    size_t m_lengthPredict; //in sizeof(type)
    int m_position;
    void * m_dataTrain = nullptr;
    void * m_dataPredict = nullptr;
    bool running;
    std::atomic<bool> dataReadyTrain;
    std::atomic<bool> dataReadyPredict;
    std::atomic<bool> inputDataTrainUseable;
    std::atomic<bool> inputDataPredictUseable;


    const uint8_t ENDPOINT_ERROR = 0;
    const uint8_t ENDPOINT_PREDICT = 1;
    const uint8_t ENDPOINT_TRAIN = 2;

    int sendGetRequest(QJsonDocument & data, QString endpoint) const; //returns http response code
    int sendPostRequest(QJsonObject & in, QJsonDocument & out, QString endpoint, bool ignoreRunningState = false) const; //returns http response code


    bool getJsonArrayFromJsonDocumentField(QJsonArray & result, QJsonDocument & response, QString field) const;
    bool getJsonArrayFromJsonObject(QJsonArray & result, QJsonObject & obj, QString field) const;
    bool getStringFromJsonDocumentField(QString & result, QJsonDocument & response, QString field) const;
    bool getBoolFromJsonDocumentField(bool & result, QJsonDocument & response, QString field) const;
    bool getIntFromJsonDocumentField(int & result, QJsonDocument & response, QString field) const;
    bool getDoubleFromJsonDocumentField(double & result, QJsonDocument & response, QString field) const;

    uint8_t getServerMode() const;
    bool setServerMode(uint8_t mode) const;
    bool getTrainingStatus(bool & trainRunning, int & epoch, double & accuracy, double & loss, double & valAccuracy, double & valLoss) const;
    bool getTrainingParams(int & epochs, int & batchSize, int & trials) const;
    bool getListOfDatasets(QMap<QString, QMap<QString, QPair<int, int>>> & datasetMap) const;
    bool getListOfX(QList<QString> & l, QString x) const;
    bool stopTraining();
    bool train();
    bool setTrainParams(int epochs = 0, int batchSize = 0, int optimizationNumOfTrials = 0);
    bool loadDataset(QString name, int fromTime = 0, int toTime = 0);
    bool loadModel(QString name, bool opti = false);
    bool deleteDataset(QString name, int fromTime = 0, int toTime = 0);
};

#endif // TAIAPICONNECTENGINEDEVICE_H
