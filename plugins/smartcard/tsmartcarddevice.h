#ifndef SMARTCARDDEVICE_H
#define SMARTCARDDEVICE_H

#include <QElapsedTimer>
#include "tiodevice.h"

#include <QQueue>
#include <winscard.h>

class TSmartCardDevice : public TIODevice {

public:

    TSmartCardDevice(QString & name, QString & info);
    TSmartCardDevice(QString & mszReader);

    virtual ~TSmartCardDevice() override;

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

    void _createPostInitParams();
    bool _validatePostInitParamsStructure(TConfigParam & params);

    bool m_createdManually;
    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;
    bool m_inputCreateAPDU;
    bool m_outputParseAPDU;
    unsigned char m_APDUHeader[4];
    unsigned char m_APDUTrailer;
    QQueue<QByteArray> m_recQueue;
    SCARDCONTEXT m_context;
    SCARDHANDLE m_card;

};

#endif // SMARTCARDDEVICE_H
