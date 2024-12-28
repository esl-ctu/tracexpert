#ifndef TTTESTDEVICE_H
#define TTTESTDEVICE_H

#include <QThread>
#include <QString>
#include <QRandomGenerator>
#include "tconfigparam.h"
#include "tanaldevice.h"
#include "typesmoment.hpp"
#include "ttest.hpp"


class TTTestDevice : public TAnalDevice {

public:
    TTTestDevice();
    virtual ~TTTestDevice();

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

    size_t addTraces(const uint8_t * buffer, size_t length, size_t classNo);
    size_t getTValues(uint8_t * buffer, size_t length, size_t class1, size_t class2, size_t order);
    void resetContexts();

private:

    TConfigParam m_preInitParams;
    size_t m_traceLength;
    size_t m_numberOfClasses;
    QString m_traceType;
    size_t m_order;

    QList<SICAK::Moments2DContext<qreal> *> m_contexts;

    QList<TAnalAction *> m_analActions;

    QList<TAnalInputStream *> m_analInputStreams;
    QList<TAnalOutputStream *> m_analOutputStreams;

    QList<QList<uint8_t> *> m_firstSet;
    QList<QList<uint8_t> *> m_secondSet;

    size_t m_length;
    int m_position;
    int * m_data = nullptr;
};

#endif // TTTESTDEVICE_H
