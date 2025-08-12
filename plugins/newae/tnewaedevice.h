
#ifndef TNEWAEDEVICE_H
#define TNEWAEDEVICE_H
#pragma once

#include "tnewae_global.h"


#include "tiodevice.h"

class TNewae;

class TnewaeDevice : public TIODevice {

public:

    TnewaeDevice(const QString & name_in, const QString & sn_in, TNewae * plugin_in, targetType type_in, bool createdManually_in = true);

    virtual ~TnewaeDevice() override;

    virtual QString getName() const override;
    virtual QString getInfo() const override;

    virtual TConfigParam getPreInitParams() const override;
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    virtual void init(bool *ok = nullptr) override;
    virtual void deInit(bool *ok = nullptr) override;

    virtual TConfigParam getPostInitParams() const override;
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual size_t writeData(const uint8_t * buffer, size_t len) override;
    virtual size_t readData(uint8_t * buffer, size_t len) override;

    void preparePreInitParams();

    void setId();
    uint8_t getId();
    QString getDeviceSn();

protected:
    //void _openPort(bool *ok = nullptr);
    TConfigParam _createPostInitParams();
    bool _validatePostInitParamsStructure(TConfigParam & params);
    TConfigParam updatePostInitParams(TConfigParam paramsIn, bool write = false) const;

    bool m_createdManually;
    //
    //

    QString sn;
    uint8_t cwId;
    QString m_name;
    QString m_info;
    TNewae * plugin;
    TnewaeScope * scopeParent;
    targetType type;

    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    qint32 m_readTimeout;
    qint32 m_writeTimeout;
    bool m_initialized;

};

#endif // TNEWAEDEVICE_H
