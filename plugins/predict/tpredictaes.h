#ifndef TPREDICTAES_H
#define TPREDICTAES_H

#include <QString>
#include <QList>
#include <QQueue>
#include "tconfigparam.h"
#include "tanaldevice.h"
#include "typesmoment.hpp"

class TPredictAES : public TAnalDevice {

public:
    TPredictAES();
    virtual ~TPredictAES();

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

    size_t addData(const uint8_t * buffer, size_t length);

    void resetContexts();
    void computePredictions();

    size_t getPredictions(uint8_t * buffer, size_t length, size_t byte);

private:

    TConfigParam m_preInitParams;

    size_t m_operation;

    QList<TAnalAction *> m_analActions;
    QList<TAnalInputStream *> m_analInputStreams;
    QList<TAnalOutputStream *> m_analOutputStreams;

    QList<uint8_t> m_data;

    QList<QList<uint8_t> *> m_predictions;
    QList<size_t> m_position;

};

#endif // TPREDICTAES_H

