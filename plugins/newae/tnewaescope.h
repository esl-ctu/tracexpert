
#ifndef TNEWAESCOPE_H
#define TNEWAESCOPE_H

#include "tnewae_global.h"
#include "tplugin.h"
//#include "tnewae.h"

//Todo správně dědit z TScope

class TnewaeScope: public TScope {
public:
    TnewaeScope(const QString & name_in, const QString & sn_in, uint8_t id_in);
    virtual ~TnewaeScope();

    virtual QString getIODeviceName() const;
    virtual QString getIODeviceInfo() const;

    virtual TConfigParam getPreInitParams() const;
    virtual TConfigParam setPreInitParams(TConfigParam params);

    virtual void init(bool *ok = nullptr);
    virtual void deInit(bool *ok = nullptr);

    virtual TConfigParam getPostInitParams() const;
    virtual TConfigParam setPostInitParams(TConfigParam params);

    /// Run the oscilloscope: wait for trigger when triggered, otherwise capture immediately
    virtual void run();
    /// Stop the oscilloscope
    virtual void stop();

    /// Downloads values from the oscilloscope, first waits for the aquisition to complete
   virtual size_t getValues(int channel, int16_t * buffer, size_t len); // CHAR8 TODO


protected:
    QString sn;
    uint8_t cwId;
    QString name;

    bool m_createdManually;

    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    qint32 m_readTimeout;
    qint32 m_writeTimeout;
    bool m_initialized;
};

#endif // TNEWAESCOPE_H
