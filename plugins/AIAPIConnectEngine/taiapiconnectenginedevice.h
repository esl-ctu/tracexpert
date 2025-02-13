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
#include <limits>

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
    size_t getData(uint8_t * buffer, size_t length);
    size_t fillData(const uint8_t * buffer, size_t length);

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

    size_t m_length; //in sizeof(type)
    int m_position;
    void * m_data = nullptr;
    std::atomic<bool> running;
    std::atomic<bool> dataReady;

    const uint8_t ENDPOINT_ERROR = 0;
    const uint8_t ENDPOINT_PREDICT = 1;
    const uint8_t ENDPOINT_TRAIN = 2;

    int sendGetRequest(QJsonDocument & data, QString endpoint); //returns http response code
    int sendPostRequest(QJsonArray & in, QJsonDocument & out, QString endpoint); //returns http response code


    bool getJsonArrayFromJsonDocumentField(QJsonArray & result, QJsonDocument & response, QString field);
    bool getJsonArrayFromJsonObject(QJsonArray & result, QJsonObject & obj, QString field);
    bool getStringFromJsonDocumentField(QString & result, QJsonDocument & response, QString field);
    bool getBoolFromJsonDocumentField(bool & result, QJsonDocument & response, QString field);
    bool getIntFromJsonDocumentField(int & result, QJsonDocument & response, QString field);
    bool getDoubleFromJsonDocumentField(double & result, QJsonDocument & response, QString field);

    uint8_t getServerMode();
    bool setServerMode(uint8_t mode);
    bool getTrainingStatus(bool & running, int & epoch, double & accuracy, double & loss, double & valAccuracy, double & valLoss);
    bool getTrainingParams(int & epochs, int & batchSize, int & trials);
    bool getListOfDatasets(QMap<QString, QPair<QString, QPair<int, int>>> & datasetMap);
    bool stopTraining();

};

#endif // TAIAPICONNECTENGINEDEVICE_H
