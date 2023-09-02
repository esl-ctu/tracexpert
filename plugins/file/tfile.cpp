#include "tfile.h"

TFile::TFile(): m_files(), m_preInitParams(), m_postInitParams() { }

TFile::~TFile() {
    (*this).TFile::deInit();
}

QString TFile::getPluginName() const {
    return QString("File");
}

QString TFile::getPluginInfo() const {
    return QString("Provides access to files.");
}


TConfigParam TFile::getPreInitParams() const {
    return m_preInitParams;
}

TConfigParam TFile::setPreInitParams(TConfigParam params) {
    m_preInitParams = params;
    return m_preInitParams;
}

void TFile::init(bool *ok) {
    if(ok != nullptr) *ok = true;
}

void TFile::deInit(bool *ok) {
    qDeleteAll(m_files.begin(), m_files.end());
    m_files.clear();
    if(ok != nullptr) *ok = true;
}

TConfigParam TFile::getPostInitParams() const {
    return m_postInitParams;
}

TConfigParam TFile::setPostInitParams(TConfigParam params) {
    m_postInitParams = params;
    return m_postInitParams;
}

TIODevice * TFile::addIODevice(QString name, QString info, bool *ok) {
    TIODevice * ret = new TFileDevice(name, info, *this);
    m_files.append(ret);
    if(ok != nullptr) *ok = true;
    return ret;
}

TScope * TFile::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
    return nullptr;
}


bool TFile::canAddIODevice() {
    return true;
}

bool TFile::canAddScope() {
    return false;
}

QList<TIODevice *> TFile::getIODevices() {
    return m_files;
}

QList<TScope *> TFile::getScopes() {
    return QList<TScope *>();
}

bool TFile::registerOpenFile(std::filesystem::path path) {
    if(m_openFilePaths.contains(path)) {
        return false;
    }
    else {
        m_openFilePaths.append(path);
        return true;
    }
}

void TFile::unregisterOpenFile(std::filesystem::path path) {
    m_openFilePaths.removeAll(path);
}

