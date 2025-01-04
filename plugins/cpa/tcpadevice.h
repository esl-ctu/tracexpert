#ifndef TCPADEVICE_H
#define TCPADEVICE_H

#include <QString>
#include <QList>
#include <QQueue>
#include "tconfigparam.h"
#include "tanaldevice.h"
#include "typesmoment.hpp"

class TCPADevice : public TAnalDevice {

public:
    TCPADevice();
    virtual ~TCPADevice();

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

    size_t addTraces(const uint8_t * buffer, size_t length);
    size_t addPredicts(const uint8_t * buffer, size_t length);

    void resetContexts();
    void computeCorrelations();

    size_t getCorrelations(uint8_t * buffer, size_t length, size_t order0);

    size_t getTypeSize(const QString & dataType);    

private:

    TConfigParam m_preInitParams;

    size_t m_traceLength;
    size_t m_predictCount;
    QString m_traceType;
    QString m_predictType;
    size_t m_order;

    SICAK::Moments2DContext<qreal> m_context;

    QList<TAnalAction *> m_analActions;
    QList<TAnalInputStream *> m_analInputStreams;
    QList<TAnalOutputStream *> m_analOutputStreams;

    QList<uint8_t> m_traces;
    QList<uint8_t> m_predicts;

    QList<SICAK::Matrix<qreal> *> m_correlations;
    QList<size_t> m_position;

};

#endif // TCPADEVICE_H

