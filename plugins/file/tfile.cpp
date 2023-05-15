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

void TFile::addIODevice(QString name, QString info, bool *ok) {
    m_files.append(new TFileDevice(name, info));
    if(ok != nullptr) *ok = true;
}

void TFile::addScope(QString name, QString info, bool *ok) {
    if(ok != nullptr) *ok = false;
}

QList<TIODevice *> TFile::getIODevices() {
    return m_files;
}

QList<TScope *> TFile::getScopes() {
    return QList<TScope *>();
}
