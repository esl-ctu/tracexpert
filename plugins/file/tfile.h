#ifndef FILE_H
#define FILE_H

#include "TFile_global.h"
#include "tplugin.h"
#include "tfiledevice.h"

#include <QFileInfo>


class TFILE_EXPORT TFile : public QObject, TPlugin
{

    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cvut.fit.TraceXpert.PluginInterface/1.0" FILE "tfile.json")
    Q_INTERFACES(TPlugin)

public:
    TFile();
    virtual ~TFile() override;

    /// Plugin name
    virtual QString getPluginName() const override;
    /// Plugin info
    virtual QString getPluginInfo() const override;

    /// Get the current pre-initialization parameters
    virtual TConfigParam getPreInitParams() const override;
    /// Set the pre-initialization parameters, returns the current params after set
    virtual TConfigParam setPreInitParams(TConfigParam params) override;

    /// Initialize the plugin
    virtual void init(bool *ok = nullptr) override;
    /// Deinitialize the plugin
    virtual void deInit(bool *ok = nullptr) override;

    /// Get the current post-initialization parameters
    virtual TConfigParam getPostInitParams() const override;
    /// Set the post-initialization parameters, returns the current params after set
    virtual TConfigParam setPostInitParams(TConfigParam params) override;

    virtual void addIODevice(QString name, QString info, bool *ok = nullptr) override;
    virtual void addScope(QString name, QString info, bool *ok = nullptr) override;

    /// Get available IO devices, available only after init()
    virtual QList<TIODevice *> getIODevices() override;
    /// Get available Scopes, available only after init()
    virtual QList<TScope *> getScopes() override;

    bool registerOpenFile(std::filesystem::path path);
    void unregisterOpenFile(std::filesystem::path path);

protected:

    QList<std::filesystem::path> m_openFilePaths;
    QList<TIODevice *> m_files;
    TConfigParam m_preInitParams;
    TConfigParam m_postInitParams;

};

#endif // FILE_H
