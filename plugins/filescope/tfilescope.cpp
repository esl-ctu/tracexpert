// TraceXpert
// Copyright (C) 2025 Embedded Security Lab, CTU in Prague, and contributors.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 
// Contributors to this file:
// Vojtěch Miškovský (initial author)

#include "tfilescope.h"

TFileScope::TFileScope()
{
    m_postInitParams = TConfigParam("File scope configuration", "", TConfigParam::TType::TDummy, "");
    m_postInitParams.addSubParam(TConfigParam("File", "", TConfigParam::TType::TFileName, "File to read traces from. File position is reset when parameters are applied."));
    m_postInitParams.addSubParam(TConfigParam("Samples per trace", "0", TConfigParam::TType::TUInt, "Number of samples per trace"));

    TConfigParam sampleType = TConfigParam("Sample data type", "Unsigned 8 bit", TConfigParam::TType::TEnum, "Data type of the samples");
    sampleType.addEnumValue("Unsigned 8 bit");
    sampleType.addEnumValue("Signed 8 bit");
    sampleType.addEnumValue("Unsigned 16 bit");
    sampleType.addEnumValue("Signed 16 bit");
    sampleType.addEnumValue("Unsigned 32 bit");
    sampleType.addEnumValue("Signed 32 bit");
    sampleType.addEnumValue("Real 32 bit (float)");
    sampleType.addEnumValue("Real 64 bit (double)");
    m_postInitParams.addSubParam(sampleType);
}

/// Scope name
QString TFileScope::getName() const
{
    return "File scope";
}

/// Scope info
QString TFileScope::getInfo() const
{
    return "Device for loading and displaying traces from file";
}

/// Get the current pre-initialization parameters
TConfigParam TFileScope::getPreInitParams() const
{
    return TConfigParam();
}

/// Set the pre-initialization parameters, returns the current params after set
TConfigParam TFileScope::setPreInitParams(TConfigParam params)
{
    return params;
}

/// Initialize the scope
void TFileScope::init(bool * ok)
{
    if (ok != nullptr) {
        *ok = true;
    }
}

/// Deinitialize the scope
void TFileScope::deInit(bool * ok)
{
    if (m_file.isOpen())
        m_file.close();

    if (ok != nullptr) {
        *ok = true;
    }
}

/// Get the current post-initialization parameters
TConfigParam TFileScope::getPostInitParams() const
{
    return m_postInitParams;
}

/// Set the post-initialization parameters, returns the current params after set
TConfigParam TFileScope::setPostInitParams(TConfigParam params)
{
    if (m_file.isOpen())
        m_file.close();

    m_file.setFileName(params.getSubParamByName(QString("File"))->getValue());
    bool open = m_file.open(QIODeviceBase::ReadOnly);

    TConfigParam * fileParam = params.getSubParamByName(QString("File"));

    if (!open)
        fileParam->setState(TConfigParam::TState::TError, "Unable to open file");
    else
        fileParam->setState(TConfigParam::TState::TOk);

    TConfigParam * samplesParam = params.getSubParamByName(QString("Samples per trace"));

    if (!samplesParam->getValue().toInt())
        samplesParam->setState(TConfigParam::TState::TError, "Samples per trace cannot be zero");
    else
        samplesParam->setState(TConfigParam::TState::TOk);

    m_postInitParams = params;
    return params;
}

/// Run the oscilloscope: wait for trigger when set, otherwise capture immediately
void TFileScope::run(size_t * expectedBufferSize, bool * ok)
{
    m_file.seek(0);

    size_t samplesPerTrace = m_postInitParams.getSubParamByName(QString("Samples per trace"))->getValue().toInt();
    size_t bytesPerSample = getTypeBytes();

    if (!m_file.isOpen() || samplesPerTrace < 1) {
        if (ok != nullptr) {
            *ok = false;
        }

        return;
    }

    *expectedBufferSize = ((size_t)m_file.bytesAvailable() / samplesPerTrace / bytesPerSample) * samplesPerTrace * bytesPerSample;

    if (ok != nullptr) {
        *ok = true;
    }
}

/// Stop the oscilloscope
void TFileScope::stop(bool * ok)
{
    if(ok != nullptr) {
        *ok = true;
    }
}

/// Downloads samples from the oscilloscope, first waits for the aquisition to complete. Works with a char memory buffer,
/// fills it with an array of samplesType values (typically will need a recast!). Returns size in bytes.
// size_t downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType & samplesType, size_t & samplesPerTraceDownloaded, size_t & tracesDownloaded) {
size_t TFileScope::downloadSamples(int channel, uint8_t * buffer, size_t bufferSize, TSampleType * samplesType, size_t * samplesPerTraceDownloaded, size_t * tracesDownloaded, bool * overvoltage)
{
    size_t samplesPerTrace = m_postInitParams.getSubParamByName(QString("Samples per trace"))->getValue().toInt();
    size_t bytesPerSample = getTypeBytes();
    size_t bytesToDownload = qMin(bufferSize, (size_t)m_file.bytesAvailable());
    size_t tracesToDownload = bytesToDownload / samplesPerTrace / bytesPerSample;

    *samplesType = getSampleType();
    *samplesPerTraceDownloaded = samplesPerTrace;
    *tracesDownloaded = tracesToDownload;
    *overvoltage = false;

    return m_file.read((char *)buffer, bufferSize);
}

TFileScope::TSampleType TFileScope::getSampleType()
{
    QString type = m_postInitParams.getSubParamByName(QString("Sample data type"))->getValue();

    if (type == "Unsigned 8 bit")
        return TFileScope::TSampleType::TUInt8;

    if (type == "Signed 8 bit")
        return TFileScope::TSampleType::TInt8;

    if (type == "Unsigned 16 bit")
        return TFileScope::TSampleType::TUInt16;

    if (type == "Signed 16 bit")
        return TFileScope::TSampleType::TInt16;

    if (type == "Unsigned 32 bit")
        return TFileScope::TSampleType::TUInt32;

    if (type == "Signed 32 bit")
        return TFileScope::TSampleType::TInt32;

    if (type == "Real 32 bit (float)")
        return TFileScope::TSampleType::TReal32;

    if (type == "Real 64 bit (double)")
        return TFileScope::TSampleType::TReal64;

    throw "Unexpected sample type set";
}

int TFileScope::getTypeBytes()
{
    QString type = m_postInitParams.getSubParamByName(QString("Sample data type"))->getValue();

    if (type == "Unsigned 8 bit" || type == "Signed 8 bit")
        return 1;

    if (type == "Unsigned 16 bit" || type == "Signed 16 bit")
        return 2;

    if (type == "Unsigned 32 bit" || type == "Signed 32 bit" || type == "Real 32 bit (float)")
        return 4;

    if (type == "Real 64 bit (double)")
        return 8;

    throw "Unexpected sample type set";
}

qsizetype TFileScope::getTypeMin()
{
    QString type = m_postInitParams.getSubParamByName(QString("Sample data type"))->getValue();

    if (type == "Unsigned 8 bit")
        return std::numeric_limits<quint8>::min();

    if (type == "Signed 8 bit")
        return std::numeric_limits<qint8>::min();

    if (type == "Unsigned 16 bit")
        return std::numeric_limits<quint16>::min();

    if (type == "Signed 16 bit")
        return std::numeric_limits<qint16>::min();

    if (type == "Unsigned 32 bit")
        return std::numeric_limits<quint32>::min();

    if (type == "Signed 32 bit")
        return std::numeric_limits<qint32>::min();

    if (type == "Real 32 bit (float)" || type == "Real 64 bit (double)")
        return -1;

    throw "Unexpected sample type set";
}

qsizetype TFileScope::getTypeMax()
{
    QString type = m_postInitParams.getSubParamByName(QString("Sample data type"))->getValue();

    if (type == "Unsigned 8 bit")
        return std::numeric_limits<quint8>::max();

    if (type == "Signed 8 bit")
        return std::numeric_limits<qint8>::max();

    if (type == "Unsigned 16 bit")
        return std::numeric_limits<quint16>::max();

    if (type == "Signed 16 bit")
        return std::numeric_limits<qint16>::max();

    if (type == "Unsigned 32 bit")
        return std::numeric_limits<quint32>::max();

    if (type == "Signed 32 bit")
        return std::numeric_limits<qint32>::max();

    if (type == "Real 32 bit (float)" || type == "Real 64 bit (double)")
        return 1;

    throw "Unexpected sample type set";
}

/// Get channel info
QList<TFileScope::TChannelStatus> TFileScope::getChannelsStatus()
{
    auto channelStatusList = QList<TChannelStatus>();
    channelStatusList.append(TChannelStatus(0, "File", true, 1, 0, getTypeMin(), getTypeMax()));
    return channelStatusList;
}

TFileScope::TTimingStatus TFileScope::getTimingStatus()
{
    return TTimingStatus(0, 0, 0, 0);
}

TFileScope::TTriggerStatus TFileScope::getTriggerStatus()
{
    return TTriggerStatus(TTriggerStatus::TTriggerType::TNone, 0, 0);
}
