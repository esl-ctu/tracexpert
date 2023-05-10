
#ifndef TNEWAESCOPE_H
#define TNEWAESCOPE_H

#include "tnewae_global.h"
#include "tplugin.h"
#include "tnewae.h"


class TnewaeScope
{
public:
    TnewaeScope(const QString & name_in, const QString & sn_in, uint8_t id_in);
    ~TnewaeScope();

    QString getScopeName() const;
    QString getScopeInfo() const;

    TConfigParam getPreInitParams() const;
    TConfigParam setPreInitParams(TConfigParam params);

    void init(bool *ok = nullptr);
    void deInit(bool *ok = nullptr);

    TConfigParam getPostInitParams() const;
    TConfigParam setPostInitParams(TConfigParam params);

    //scope functions:


protected:
    QString sn;
    uint8_t cwId;
    QString name;

    /*void _createPostInitParams();
    bool _validatePostInitParamsStructure(TConfigParam & params);*/

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
