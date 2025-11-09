#ifndef TPRESENTENGINE_H
#define TPRESENTENGINE_H

#include <QString>
#include <QList>
#include <QQueue>
#include "tconfigparam.h"
#include "tanaldevice.h"

class TPRESENTEngine : public TAnalDevice {

public:
    TPRESENTEngine();
    virtual ~TPRESENTEngine();

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
    size_t addKeyData(const uint8_t * buffer, size_t length);

    void reset();
    void computeIntermediates();
    void loadKey();

    size_t getIntermediates(uint8_t * buffer, size_t length);
    size_t availableBytes();

private:

    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;

    size_t m_operation;
    size_t m_keysizeB;

    QList<TAnalAction *> m_analActions;
    QList<TAnalInputStream *> m_analInputStreams;
    QList<TAnalOutputStream *> m_analOutputStreams;

    QList<uint8_t> m_data;
    QList<uint8_t> m_keyData;

    QList<uint8_t> m_intermediates;
    size_t m_position;

    size_t m_breakpoint;
    size_t m_breakpointN;

    size_t m_outputRestrict;
    size_t m_outputRestrictM;

    uint8_t m_key[16];

};

#endif // TPRESENTENGINE_H

