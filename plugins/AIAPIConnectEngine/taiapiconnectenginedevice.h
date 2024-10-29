#ifndef TAIAPICONNECTENGINEDEVICE_H
#define TAIAPICONNECTENGINEDEVICE_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QUrl>

#include "tconfigparam.h"
#include "tanaldevice.h"


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

    //size_t fillData(const uint8_t * buffer, size_t length, QList<QList<uint8_t> *> & set);
    //void processData(bool subtract);
    //size_t getData(uint8_t * buffer, size_t length);

private:

    TConfigParam m_preInitParams;
    size_t m_traceLength;

    QList<TAnalAction *> m_analActions;

    QList<TAnalInputStream *> m_analInputStreams;
    QList<TAnalOutputStream *> m_analOutputStreams;


    size_t m_length;
    int m_position;
    int * m_data = nullptr;

};

#endif // TAIAPICONNECTENGINEDEVICE_H
