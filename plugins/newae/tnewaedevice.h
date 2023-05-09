
#ifndef TNEWAEDEVICE_H
#define TNEWAEDEVICE_H

#include "tiodevice.h"

class TnewaeDevice : public TIODevice {

public:

    TnewaeDevice(const QString & name, const QString & sn);
    //TnewaeDevice(const QSerialPortInfo &portInfo);

    virtual ~TnewaeDevice() override;

    virtual QString getIODeviceName() const override;
    virtual QString getIODeviceInfo() const override;

    virtual TConfigParam getPreInitParams() const override;
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    virtual void init(bool *ok = nullptr) override;
    virtual void deInit(bool *ok = nullptr) override;

    virtual TConfigParam getPostInitParams() const override;
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual size_t writeData(const uint8_t * buffer, size_t len) override;
    virtual size_t readData(uint8_t * buffer, size_t len) override;

protected:
    QString sn;
    int cwId;

    /*void _openPort(bool *ok = nullptr);
    void _createPostInitParams();
    bool _validatePostInitParamsStructure(TConfigParam & params);*/

    bool m_createdManually;
    //
    //
    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    qint32 m_readTimeout;
    qint32 m_writeTimeout;
    bool m_initialized;

};

#endif // TNEWAEDEVICE_H
