#ifndef RANDOMDEVICE_H
#define RANDOMDEVICE_H

#include <QElapsedTimer>
#include "tiodevice.h"
#include <functional>
#include <random>

#define RANDOM_GENERATOR_TYPE std::mt19937_64

class TRandomDevice : public TIODevice {

public:

    TRandomDevice(QString & name, QString & info);

    virtual ~TRandomDevice() override;

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
    virtual std::optional<size_t> availableBytes() override;

protected:

    void _createPreInitParams();
    void _createPostInitParams();
    bool _validatePreInitParamsStructure(TConfigParam & params);
    bool _validatePostInitParamsStructure(TConfigParam & params);
    bool _validatePostInitParamsValues(TConfigParam & params);

    bool _setDistribution(QString distributionName, TConfigParam::TType dataType, QString parameter1 = "", QString  parameter2 = "");

    template<typename T>
    void _setDistributionFunc(T distribution);


    QString m_name;
    QString m_info;

    RANDOM_GENERATOR_TYPE m_engine;
    std::function<void(RANDOM_GENERATOR_TYPE&, uint8_t *)> m_distribution_func;

    TConfigParam::TType m_returnType;
    quint8 m_returnTypeSize;

    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_preInitParamsValid;
    bool m_initialized;

};

#endif // RANDOMDEVICE_H
