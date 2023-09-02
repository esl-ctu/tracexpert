#ifndef FILEDEVICE_H
#define FILEDEVICE_H

#include <QFileInfo>
#include <QFile>
#include <QElapsedTimer>
#include "tiodevice.h"

class TFile;

class TFileDevice : public TIODevice {

public:

    TFileDevice(QString & name, QString & info, TFile & tFile);

    virtual ~TFileDevice() override;

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

    void _openFile(bool *ok = nullptr);
    void _createPreInitParams();
    void _createPostInitParams();
    bool _validatePreInitParamsStructure(TConfigParam & params);
    bool _validatePostInitParamsStructure(TConfigParam & params);

    bool m_createdManually;
    QFlags<QIODevice::OpenModeFlag> m_openMode;
    QFile m_file;
    QFileInfo m_fileInfo;
    QString m_name;
    QString m_info;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;
    bool m_initialized;

    TFile & m_tFile;
};

#endif // FILEDEVICE_H
